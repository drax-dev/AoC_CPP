#include <iostream>
#include <filesystem>
#include <fstream>
#include <map>
#include <queue>
#include <sstream>
#include <string>

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
    std::ifstream f(path, std::ios::in | std::ios::binary);
    const auto sz = fs::file_size(path);
    std::string result(sz, '\0');
    f.read(result.data(), static_cast<long long>(sz));
    return result;
}

using graph = std::map<std::string, std::vector<std::string>>;

graph build_graph(const std::string& input_data)
{
	graph graph_instance;
    std::string first;
	std::string second;
    std::stringstream ss(input_data);
	while (std::getline(ss, first, '-')) 
    {
        std::getline(ss, second);
        second = trim(second);
		if(second != "start")
		{
			graph_instance[first].push_back(second);
		}
		if(first != "start")
		{
			graph_instance[second].push_back(first);
		}
    }
	graph_instance["end"].clear();

    return graph_instance;
}

using visited_map = std::map<std::string, int>;
using paths_map = std::map<int, std::string>;

int traverse_all_path(const std::string& from, const std::string& to, visited_map& visited_nodes, graph& graph_instance,
	paths_map& path, int path_index)
{
	int result = 0;
	visited_nodes[from] += 1;
	path[path_index] = from;
	path_index++;
	const auto node_neighbors = graph_instance[from];

	if (from == to) 
	{
		for (int i = 0; i < path_index; i++)
		{
			std::cout << path[i] << " ";
		}
		std::cout << std::endl;
		result += 1;
	}
	else
	{
		for (const auto& neighbor : node_neighbors)
		{
			if (visited_nodes[neighbor] < 1 || std::isupper(neighbor[0]))
			{
				result += traverse_all_path(neighbor, to, visited_nodes, graph_instance, path, path_index);
			}
		}
	}

	path_index--;
	path.erase(path_index);
	visited_nodes[from] -= 1;
	return result;
}

bool is_any_node_visited_twice(const visited_map& visited_nodes)
{
	return std::any_of(visited_nodes.begin(), visited_nodes.end(), [](const std::pair<const std::string, int>& pair)
	{
		if (std::islower(pair.first[0]))
		{
			return pair.second > 1;
		}
		return false;
	});
}

int traverse_all_path_2(const std::string& from, const std::string& to, visited_map& visited_nodes, graph& graph_instance,
	paths_map& path, int path_index)
{
	int result = 0;
	visited_nodes[from] += 1;
	path[path_index] = from;
	path_index++;
	const auto &node_neighbors = graph_instance[from];

	if (from == to) 
	{
		//for (int i = 0; i < path_index; i++)
		//{
		//	std::cout << path[i] << " ";
		//}
		//std::cout << std::endl;
		result += 1;
	}
	else
	{
		for (const auto& neighbor : node_neighbors)
		{
			if (visited_nodes[neighbor] < 1 || std::isupper(neighbor[0]) || !is_any_node_visited_twice(visited_nodes))
			{
				result += traverse_all_path_2(neighbor, to, visited_nodes, graph_instance, path, path_index);
			}
		}
	}

	path_index--;
	path.erase(path_index);
	visited_nodes[from] -= 1;
	return result;
}

void print_all_paths(graph& graph_instance, const std::string& from, const std::string& to)
{
	std::queue<std::string> node_queue;
	visited_map visited_nodes;
	paths_map paths;
	constexpr int path_index = 0;

	const auto result = traverse_all_path(from, to, visited_nodes, graph_instance, paths, path_index);
	std::cout << "Number of paths = " << result << std::endl;
}

void print_all_paths_2(graph& graph_instance, const std::string& from, const std::string& to)
{
	std::queue<std::string> node_queue;
	visited_map visited_nodes;
	visited_map visited_twice_nodes;
	paths_map paths;
	constexpr int path_index = 0;

	const auto result = traverse_all_path_2(from, to, visited_nodes, graph_instance, paths, path_index);
	std::cout << "Number of paths = " << result << std::endl;
}

int main()
{
	// part 1
	auto result = read_file("input_day12.txt");
    auto graph_instance = build_graph(result);
	print_all_paths(graph_instance, "start", "end");

	// part 2
	result = read_file("input_day12.txt");
    graph_instance = build_graph(result);
	print_all_paths_2(graph_instance, "start", "end");
}