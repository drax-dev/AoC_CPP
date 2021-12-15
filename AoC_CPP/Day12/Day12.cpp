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
		if(first != "start" && first != "end")
		{
			graph_instance[second].push_back(first);
		}
    }
	graph_instance["end"].clear();

    return graph_instance;
}

void dfs(std::map<std::string, bool>& visited_nodes, graph& graph_instance, const std::string& node)
{
    if (!visited_nodes[node])
    {
    	std::cout << node << ", ";
		visited_nodes[node] = true;
		const auto node_neighbors = graph_instance[node];
		for (const auto& neighbor : node_neighbors)
		{
			dfs(visited_nodes, graph_instance, neighbor);
		}
	}
}

using visitedMap = std::map<std::string, bool>;
using pathsMap = std::map<int, std::string>;

int traverse_all_path(const std::string& from, const std::string& to, visitedMap& visited_nodes, graph& graph_instance,
	pathsMap& path, int path_index)
{
	int result = 0;
	visited_nodes[from] = true;
	path[path_index] = from;
	path_index++;

	if (from == to) 
	{
		for (int i = 0; i < path_index; i++)
			std::cout << path[i] << " ";
		std::cout << std::endl;
		result += 1;
	}
	else // If current vertex is not destination
	{
		const auto node_neighbors = graph_instance[from];
		for (const auto& neighbor : node_neighbors)
			if (!visited_nodes[neighbor] || std::isupper(neighbor[0]))
			{
				result += traverse_all_path(neighbor, to, visited_nodes, graph_instance, path, path_index);
			}
	}

	// Remove current vertex from path[] and mark it as unvisited
	path_index--;
	visited_nodes[from] = false;
	return result;
}

void print_all_paths(graph& graph_instance, const std::string& from, const std::string& to)
{
	std::queue<std::string> node_queue;
	visitedMap visited_nodes;
	pathsMap paths;
	int path_index = 0;

	const auto result = traverse_all_path(from, to, visited_nodes, graph_instance, paths, path_index);
	std::cout << "Number of paths = " << result << std::endl;
}


int main()
{
	const auto result = read_file("input_day12_larger.txt");
    auto graph_instance = build_graph(result);
	print_all_paths(graph_instance, "start", "end");
    std::cout<< "";
}