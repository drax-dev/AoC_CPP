#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

struct point
{
	point(const int y, const int x) :
		x(x), y(y)
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
		//std::cout << first << "," << second << std::endl;
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

		//std::cout << first << "=" << second << std::endl;
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
	board board_instance(max_x + 1, std::vector(max_y + 1, '.'));
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

int count_dots(const board& board_instance)
{
	auto dots_counter = 0;
	for (const auto& board_line : board_instance)
	{
		for (const auto board_field : board_line)
		{
			if (board_field == '#')
			{
				dots_counter++;
			}
		}
	}
	return dots_counter;
}

board fold_board(const board& board_instance, const point& fold)
{
	board result;
	if (fold.y == 0)
	{
		result = board(fold.x, std::vector(board_instance[0].size(), '.'));
		for (int x = 0; x < static_cast<int>(board_instance.size()); x++)
		{
			for (int y = 0; y < static_cast<int>(board_instance[x].size()); y++)
			{
				if (board_instance[x][y] == '#')
				{
					if (x > fold.x)
					{
						const auto diff = x - fold.x;
						const auto new_index = fold.x - diff;
						result[new_index][y] = board_instance[x][y];
					}
					else if (x < fold.x)
					{
						result[x][y] = board_instance[x][y];
					}
		
				}
			}
		}
	}
	else
	{
		result = board(board_instance.size(), std::vector(fold.y, '.'));
		for (int x = 0; x < static_cast<int>(board_instance.size()); x++)
		{
			for (int y = 0; y < static_cast<int>(board_instance[x].size()); y++)
			{
				if (board_instance[x][y] == '#')
				{
					if (y > fold.y)
					{
						const auto diff = y - fold.y;
						const auto new_index = fold.y - diff;
						result[x][new_index] = board_instance[x][y];
					}
					else if (y < fold.y)
					{
						result[x][y] = board_instance[x][y];
					}
				}
			}
		}
	}
	return result;
}

int main()
{
	// part 1
	const auto result = read_file("input_day13.txt");
	const auto [points, folds] = parse_input(result);
	auto board = create_board(points);
	//print_board(board);
	std::cout << '\n';
	for (const auto& fold : folds)
	{
		board = fold_board(board, fold);
		std::cout << '\n';
		//print_board(board);
		const auto dots_counter = count_dots(board);
		std::cout << '\n';
		std::cout << dots_counter;
		std::cout << '\n';
	}

	// part 2
	print_board(board);
}
