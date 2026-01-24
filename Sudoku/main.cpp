#include <iostream>
#include <fstream>
#include "Sudoku.h"

int main()
{
    Sudoku s;
    //s.loadFromFile("hardest1.txt");
	std::ifstream in("hardest1.txt");
	//std::cin >> s;
	in >> s;
	std::cout << s;

	if(s.validate())
		std::cout << "The Sudoku puzzle is valid." << std::endl;
	else
		std::cout << "The Sudoku puzzle is invalid." << std::endl;
	std::cout << "Size of sudoku: " << sizeof(Sudoku) << " bytes" << std::endl;
	return 0;
}