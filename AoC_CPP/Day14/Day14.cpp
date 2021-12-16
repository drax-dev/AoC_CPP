#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>

namespace fs = std::filesystem;

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

std::pair<std::string, std::unordered_map<std::string, char>> parse_input(const std::string& input_data)
{
	std::string polymer_template;
    std::stringstream ss(input_data);
	std::getline(ss, polymer_template);
	polymer_template = trim(polymer_template);
	//std::cout << polymer_template << std::endl;

	std::string first;
	std::string second;
	std::unordered_map<std::string, char> rules;
	while (std::getline(ss, first, '-')) 
    {
		first = trim(first);
        std::getline(ss, second);
		second = second.substr(1);
		second = trim(second);
		rules[first] = second[0];
		//std::cout << first << "->" << second << std::endl;
    }
	return std::make_pair(polymer_template, rules);
}

void build_polymer(std::unordered_map<std::string, int>& occurrences, const std::unordered_map<std::string, char>& rules, const int number_of_steps)
{
	for (int i = 0; i < number_of_steps; i++)
	{
		std::unordered_map<std::string, int> new_occurrences;
		for (auto& [pair, number_of_occurrences] : occurrences)
		{
			while(number_of_occurrences > 0)
			{
				const auto new_polymer = rules.at(pair);
				number_of_occurrences--;
				std::string new_pair;
				new_pair += pair[0];
				new_pair += new_polymer;
				new_occurrences[new_pair]++;
				new_pair.clear();
				new_pair += new_polymer;
				new_pair += pair[1];
				new_occurrences[new_pair]++;
			}
		}

		for (const auto& [pair, number_of_occurrences] : new_occurrences)
		{
			occurrences[pair] += number_of_occurrences;
		}

		unsigned long long int polymer_length = 1;
		for (const auto& [pair, number_of_occurrences] : occurrences)
		{
			polymer_length += number_of_occurrences;
		}
		std::cout << "Step " << i + 1 <<  " polymer length=" << polymer_length << std::endl;
	}
}

void build_polymer_optimal(std::unordered_map<std::string, unsigned long long int>& occurrences, const std::unordered_map<std::string, char>& rules, const int number_of_steps)
{
	for (int i = 0; i < number_of_steps; i++)
	{
		std::unordered_map<std::string, unsigned long long int> new_occurrences;
		for (auto& [pair, number_of_occurrences] : occurrences)
		{
			if(number_of_occurrences > 0)
			{
				const auto new_polymer = rules.at(pair);
				std::string new_pair;
				new_pair += pair[0];
				new_pair += new_polymer;
				new_occurrences[new_pair] += number_of_occurrences;
				new_pair.clear();
				new_pair += new_polymer;
				new_pair += pair[1];
				new_occurrences[new_pair] += number_of_occurrences;
				number_of_occurrences = 0;
			}
		}

		for (const auto& [pair, number_of_occurrences] : new_occurrences)
		{
			occurrences[pair] += number_of_occurrences;
		}

		unsigned long long int polymer_length = 1;
		for (const auto& [pair, number_of_occurrences] : occurrences)
		{
			polymer_length += number_of_occurrences;
		}
		std::cout << "Step " << i + 1 <<  " polymer length=" << polymer_length << std::endl;
	}
}


int main()
{
	// part 1
	{
		const auto result = read_file("input_day14_small.txt");
		const auto [template_polymers, rules] = parse_input(result);
		std::unordered_map<std::string, int> occurrences;
		for (const auto& [pair, polymer] : rules)
		{
			occurrences[pair] = 0;
		}
		for (int i = 1; i < template_polymers.size(); i++)
		{
			std::string pair;
			pair += template_polymers[i - 1];
			pair += template_polymers[i];
			occurrences[pair]++;
		}

		build_polymer(occurrences, rules, 10);

		std::unordered_map<char, int> single_element_occurrences;
		for (const auto& [polymer_pair, number_of_occurrences] : occurrences)
		{
			for (const auto character : polymer_pair)
			{
				single_element_occurrences[character] += number_of_occurrences;
			}
		}
		unsigned long long int min_value = std::numeric_limits<int>::max();
		unsigned long long int max_value = 0;
		for (const auto& [character, number_of_occurrences] : single_element_occurrences)
		{
			min_value = std::min(min_value, static_cast<unsigned long long int>(number_of_occurrences));
			max_value = std::max(max_value, static_cast<unsigned long long int>(number_of_occurrences));
		}
		std::cout << (min_value + 1) / 2 << " " << (max_value + 1) / 2 << std::endl;
		const unsigned long long int final_result = ((max_value + 1) / 2) - ((min_value + 1) / 2);
		std::cout << final_result << std::endl;
	}

	// part2
	{
		const auto result = read_file("input_day14.txt");
		const auto [template_polymers, rules] = parse_input(result);
		std::unordered_map<std::string, unsigned long long int> occurrences;
		for (const auto& [pair, polymer] : rules)
		{
			occurrences[pair] = 0;
		}
		for (int i = 1; i < template_polymers.size(); i++)
		{
			std::string pair;
			pair += template_polymers[i - 1];
			pair += template_polymers[i];
			occurrences[pair]++;
		}

		build_polymer_optimal(occurrences, rules, 40);

		std::unordered_map<char, unsigned long long int> single_element_occurrences;
		for (const auto& [polymer_pair, number_of_occurrences] : occurrences)
		{
			for (const auto character : polymer_pair)
			{
				single_element_occurrences[character] += number_of_occurrences;
			}
		}
		unsigned long long int min_value = std::numeric_limits<unsigned long long int>::max();
		unsigned long long int max_value = 0;
		for (const auto &[character, number_of_occurrences] : single_element_occurrences)
		{
			min_value = std::min(min_value, number_of_occurrences);
			max_value = std::max(max_value, number_of_occurrences);
		}
		min_value = (min_value + 1) / 2;
		max_value = (max_value + 1) / 2;
		std::cout << min_value << " " << max_value << std::endl;
		const unsigned long long int final_result = max_value - min_value;
		std::cout << final_result << std::endl;
	}
}
