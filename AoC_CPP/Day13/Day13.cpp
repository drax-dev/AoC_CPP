#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

struct point
{
	point(const int y, const int x):
	y(y), x(x)
	{ }

	int x;
	int y;
};

std::string trim(std::string string_input)
{
	constexpr auto whitespaces = " \t\n\r\f\v";
	string_input.erase(string_input.find_last_not_of(whitespaces) + 1);
	string_input.erase(0,string_input.find_first_not_of(whitespaces));
	return string_input;
}

std::string read_file(const fs::path& path)
{
	try
	{
		std::ifstream f(path, std::ios::in | std::ios::binary);
		const auto sz = fs::file_size(path);
		std::string result(sz, '\0');
		f.read(result.data(), static_cast<long long>(sz));
		return result;
	}
	 catch(fs::filesystem_error& e) 
	 {
        std::cout << e.what() << '\n';
    }
	return {};
}

using board = std::vector<std::vector<char>>;

std::pair<std::vector<point>, std::vector<point>> parse_input(const std::string& input_data)
{
	std::string first;
	std::string second;
    std::stringstream ss(input_data);
	std::string second_part;
	std::vector<point> points;

	while (std::getline(ss, first, ',')) 
    {
		if(first[0] == '\r')
		{
			second_part = first;
			break;
		}
        std::getline(ss, second);
        second = trim(second);
		const auto first_number = std::stoi(first);
		const auto second_number = std::stoi(second);
		points.emplace_back(first_number, second_number);
		std::cout << first << "," << second << std::endl;
    }

	std::vector<point> folds;
	std::stringstream second_part_ss(second_part);
	while (std::getline(second_part_ss, first, '=')) 
    {
        std::getline(second_part_ss, second);
        first = trim(first);
		first = first[first.size() - 1];
        second = trim(second);

		const auto number = std::stoi(second);
		if(first == "x")
		{

			folds.emplace_back(number, 0);
		}
		else
		{
			folds.emplace_back(0, number);
		}

		std::cout << first << "=" << second << std::endl;
    }

	return std::make_pair(points, folds);
}

board create_board(const std::vector<point>& points)
{
	int max_x = 0;
	int max_y = 0;
	for (const auto& point : points)
	{
		max_x = std::max(point.x, max_x);
		max_y = std::max(point.y, max_y);
	}
	board board_instance(max_x + 1, std::vector<char>(max_y + 1, '.'));
	for (const auto& point : points)
	{
		board_instance[point.x][point.y] = '#';
	}

	return board_instance;
}

void print_board(const board& board_instance)
{
	for (const auto& board_line : board_instance)
	{
		for (const auto board_field : board_line)
		{
			std::cout << board_field << " ";
		}
		std::cout << '\n';
	}
}

int main()
{
	// part 1
	auto result = read_file("input_day13_small.txt");
	auto points_and_folds = parse_input(result);
	auto board = create_board(points_and_folds.first);
	print_board(board);
	std::cout << "";
	// part 2
}
