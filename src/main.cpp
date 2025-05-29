#include <iostream>
#include <boost/asio.hpp>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <boost/asio.hpp>
#include <fstream>
#include <mutex>
#include <unordered_map>


#pragma pack(push, 1)
struct LogRecord {
    char sensor_id[32];
    std::time_t timestamp;
    double value;
};
#pragma pack(pop)

std::time_t string_to_time_t(const std::string& time_string) {
    std::tm tm = {};
    std::istringstream ss(time_string);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    return std::mktime(&tm);
}

std::string time_t_to_string(std::time_t time) {
    std::tm* tm = std::localtime(&time);
    std::ostringstream ss;
    ss << std::put_time(tm, "%Y-%m-%dT%H:%M:%S");
    return ss.str();
}

using boost::asio::ip::tcp;

// Mutex por sensor para controlar acesso ao arquivo
std::unordered_map<std::string, std::mutex> file_mutexes;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket)
        : socket_(std::move(socket)) {}

    void start() {
        read_message();
    }
    
private:
    void read_message() {
        auto self(shared_from_this());
        boost::asio::async_read_until(socket_, buffer_, "\r\n",
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    std::istream is(&buffer_);
                    std::string message;
                    std::getline(is, message);
                    if (!message.empty() && message.back() == '\r') {
                        message.pop_back(); // remover \r
                    }
                    process_message(message);
                }
            });
    }

    void process_message(const std::string& message) {
        if (message.rfind("LOG|", 0) == 0) {
            handle_log(message);
        } else if (message.rfind("GET|", 0) == 0) {
            handle_get(message);
        } else {
            send_message("ERROR|INVALID_FORMAT\r\n");
        }
    }

    void handle_log(const std::string& message) {
        // Exemplo: LOG|SENSOR_001|2023-05-11T15:30:00|78.5
        std::istringstream ss(message);
        std::string command, sensor_id, datetime_str, value_str;
        getline(ss, command, '|');
        getline(ss, sensor_id, '|');
        getline(ss, datetime_str, '|');
        getline(ss, value_str);

        std::time_t timestamp = string_to_time_t(datetime_str);
        double value = std::stod(value_str);

        LogRecord record;
        std::strncpy(record.sensor_id, sensor_id.c_str(), sizeof(record.sensor_id));
        record.timestamp = timestamp;
        record.value = value;

        std::string filename = sensor_id + ".bin";

        {
            std::lock_guard<std::mutex> lock(file_mutexes[sensor_id]);
            std::ofstream ofs(filename, std::ios::binary | std::ios::app);
            ofs.write(reinterpret_cast<char*>(&record), sizeof(LogRecord));
        }

        // Pronto para próxima leitura
        read_message();
    }

    void handle_get(const std::string& message) {
        // Exemplo: GET|SENSOR_001|10
        std::istringstream ss(message);
        std::string command, sensor_id, n_str;
        getline(ss, command, '|');
        getline(ss, sensor_id, '|');
        getline(ss, n_str);

        int n = std::stoi(n_str);
        std::string filename = sensor_id + ".bin";

        std::vector<LogRecord> records;

        {
            std::lock_guard<std::mutex> lock(file_mutexes[sensor_id]);
            std::ifstream ifs(filename, std::ios::binary);
            if (!ifs) {
                send_message("ERROR|INVALID_SENSOR_ID\r\n");
                return;
            }

            ifs.seekg(0, std::ios::end);
            std::streamsize size = ifs.tellg();
            int total_records = size / sizeof(LogRecord);
            int start_index = std::max(0, total_records - n);

            ifs.seekg(start_index * sizeof(LogRecord), std::ios::beg);
            for (int i = start_index; i < total_records; ++i) {
                LogRecord rec;
                ifs.read(reinterpret_cast<char*>(&rec), sizeof(LogRecord));
                records.push_back(rec);
            }
        }

        std::ostringstream response;
        response << records.size();
        for (const auto& rec : records) {
            response << ";" << time_t_to_string(rec.timestamp) << "|" << rec.value;
        }
        response << "\r\n";

        send_message(response.str());
    }

    void send_message(const std::string& msg) {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(msg),
            [this, self](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    read_message();
                }
            });
    }

    tcp::socket socket_;
    boost::asio::streambuf buffer_;
};

class Server {
public:
    Server(boost::asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
        accept();
    }

private:
    void accept() {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::make_shared<Session>(std::move(socket))->start();
                }
                accept(); // Continua aceitando novas conexões
            });
    }

    tcp::acceptor acceptor_;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: ./servidor <porta>\n";
        return 1;
    }

    try {
        boost::asio::io_context io_context;
        Server server(io_context, std::atoi(argv[1]));
        std::cout << "Servidor iniciado na porta " << argv[1] << std::endl;
        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Erro: " << e.what() << std::endl;
    }

    return 0;
}
