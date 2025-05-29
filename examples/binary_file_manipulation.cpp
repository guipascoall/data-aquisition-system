#include <iostream>
#include <fstream>

struct record_t
{
	 int  id;
	 char name[16];
	 char phone[16]; // 3134679819
};

void gen_random_name(char *s, const int len = 15) 
{
	static const char alphanum[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	for (int i = 0; i < len; ++i) {
		s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
	}
	s[len] = 0;
}

void gen_random_phone(char *s, const int len = 9) 
{
	static const char alphanum[] ="0123456789";

	for (int i = 0; i < len; ++i) 
	{
		if (i==4)
			s[i] = '-';
		else
			s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
	}
	s[len] = 0;
}

int main()
{
	// Abre o arquivo para leitura e escrita em modo binário e coloca o apontador do arquivo
	// apontando para o fim de arquivo
	std::fstream file("phonebook.dat", std::fstream::out | std::fstream::in | std::fstream::binary 
																	 | std::fstream::app); 
	// Caso não ocorram erros na abertura do arquivo
	if (file.is_open())
	{
		// Imprime a posição atual do apontador do arquivo (representa o tamanho do arquivo)
		int file_size = file.tellg();

		// Recupera o número de registros presentes no arquivo
		int n = file_size/sizeof(record_t);
		std::cout << "Num records: " << n << " (file size: " << file_size << " bytes)" << std::endl;

		// Escreve 10 registros no arquivo
		std::cout << "Writing 10 more records..." << std::endl;
		int id = n+1;
		for (unsigned i =0; i < 10; ++i)
		{
			record_t rec;
			rec.id = id++;
			gen_random_name(rec.name);
			gen_random_phone(rec.phone);
			file.write((char*)&rec, sizeof(record_t));
		}

		// Imprime a posição atual do apontador do arquivo (representa o tamanho do arquivo)
		file_size = file.tellg();
		// Recupera o número de registros presentes no arquivo
		n = file_size/sizeof(record_t);
		std::cout << "Num records: " << n << " (file size: " << file_size << " bytes)" << std::endl;
		
		bool id_ok = false;
		while (!id_ok)
		{
			std::cout << "Id: ";
			std::cin >> id;
			if (id > n)
				std::cout << "Invalid id" << std::endl;
			else
				id_ok = true;
		}
		file.seekp((id-1)*sizeof(record_t), std::ios_base::beg);

		// Le o registro selecionado
		record_t rec;
		file.read((char*)&rec, sizeof(record_t));

		// Imprime o registro
		std::cout << "Id: "  << rec.id << " - Name: " << rec.name << " - Phone: "<< rec.phone << std::endl;

		// Fecha o arquivo
		file.close();
	}
	else
	{
		std::cout << "Error opening file!" << std::endl;
	}
	return(0);
}
