#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <unordered_map>

namespace fs = std::filesystem;

struct point
{
	point() : x(0), y(0), cost(0), priority(0)
	{
	}

	point(const int y, const int x, const int cost) :
		x(x), y(y), cost(cost)
	{ 
	}

	int x;
	int y;
	int cost;
	int priority;

	bool operator==(const point& other) const
	{
	  return this->x == other.x && this->y == other.y && this->cost == other.cost;
	}
};

  template <>
  struct std::hash<point>
  {
    std::size_t operator()(const point& p) const
    {
      // Compute individual hash values for first,
      // second and third and combine them using XOR
      // and bit shifting:

      return ((std::hash<int>()(p.x)
               ^ (std::hash<int>()(p.y) << 1)) >> 1)
               ^ (std::hash<int>()(p.cost) << 1);
    }
  };

bool isSafe(int x, int y, int max_x, int max_y){
	return x>=0 && x< max_x && y>=0 && y< max_y;
}

std::vector<std::pair<int, int>> moves{{1,0},{-1,0},{0,1},{0,-1}};

std::vector<point> get_neighbors(point p, const std::vector<std::vector<point>>& input_data)
{
	std::vector<point> neighbors;
	for(auto& [x,y] : moves)
	{
		if (isSafe(p.x + x , p.y + y, input_data.size(),input_data[0].size()))
		{
			neighbors.push_back(input_data[p.x + x][p.y + y]);
		}
	}
	return neighbors;
}

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

std::vector<std::vector<point>> parse_input(const std::string& input_data)
{
	std::vector<std::vector<point>> input;
	std::stringstream ss(input_data);
	std::string line;

	auto line_counter = 0;
	while (std::getline(ss, line))
	{
		std::vector<point> transformed_line;
		line = trim(line);
		for (auto i  = 0; i < line.size(); i++)
		{
			const auto cost = line[i] - '0';
			transformed_line.emplace_back(i, line_counter, cost);
		}
		input.push_back(transformed_line);
		line_counter++;
	}

	return input;
}

inline int heuristic(const point a, const point b)
{
  return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

struct compareStruct
{
    bool operator() (const point& lhs, const point& rhs) const
    {
        return lhs.priority > rhs.priority;
    }
};

void a_star_search(const std::vector<std::vector<point>>& input_data, const point start, const point goal, std::unordered_map<point, point>& came_from, std::unordered_map<point, int>& cost_so_far)
{
	std::priority_queue<point, std::vector<point>, compareStruct> frontier;
	frontier.push(start);

	came_from[start] = start;
	cost_so_far[start] = 0;

	while (!frontier.empty())
	{
		point current = frontier.top();
		frontier.pop();

		if (current == goal)
		{
			break;
		}

		for (point next : get_neighbors(current, input_data))
		{
			const int new_cost = cost_so_far[current] + next.cost;
			if (cost_so_far.find(next) == cost_so_far.end() || new_cost < cost_so_far[next])
			{
				cost_so_far[next] = new_cost;
				const int priority = new_cost + heuristic(next, goal);
				next.priority = priority;
				frontier.push(next);
				came_from[next] = current;	
			}
		}
	}
}


void extend_row(std::vector<point> vec, std::vector<point>& input_elem, const int size_y)
{
    for (int i = 1; i < 5; i++) 
	{
        std::transform(std::begin(vec), std::end(vec), std::begin(vec), [&](point x) 
			{
	        const int modulo = (x.cost + 1) % 9;
            x.cost = modulo == 0 ? 9 : modulo;
			//x.y = (x.y) % 10 + i * size_y;
			x.y += size_y;
            return x; });
        input_elem.insert(input_elem.end(), vec.begin(), vec.end());
    }
}

void increase_row(std::vector<std::vector<point>>& vec, std::vector<point> input_elem, const int i, const int size_x)
{
    std::transform(std::begin(input_elem), std::end(input_elem), std::begin(input_elem), [&](point x) 
		{
	    const int modulo = (x.cost + i) % 9;
        x.cost = modulo == 0 ? 9 : modulo;
		x.x += i * size_x;
        return x; });
    vec.push_back(input_elem);
}

std::vector<std::vector<point>> extendInput(std::vector<std::vector<point>> input_copy, const int size_x, const int size_y)
{
    for (auto& input_elem : input_copy) 
	{
    	std::vector<point> vec = input_elem;
		//std::copy(input_elem.begin(), input_elem.end(), std::back_inserter(vec));
        extend_row(vec, input_elem, size_y);
    }

    const std::vector<std::vector<point>> vec = input_copy;
    for (int i = 1; i < 5; i++) 
	{
        for (auto& input_elem : vec) 
		{
            increase_row(input_copy, input_elem, i, size_x);
        }
    }
    return input_copy;
}

int main()
{
	auto input = read_file("input_day15.txt");

	int size;
	// part 1
	{
		auto parsed_input = parse_input(input);
		parsed_input[0][0].cost = 0; //First point has no cost ! 
		std::unordered_map<point, point> came_from;
		std::unordered_map<point, int> cost_so_far;
		a_star_search(parsed_input, parsed_input[0][0], parsed_input[parsed_input.size() - 1][parsed_input[0].size() - 1], came_from, cost_so_far);
		std::cout << cost_so_far[parsed_input[parsed_input.size() - 1][parsed_input[0].size() - 1]] << std::endl;
	}

	// part 2
	{
		auto parsed_input = parse_input(input);
		auto size_x = parsed_input.size();
		auto size_y = parsed_input[0].size();
		parsed_input = extendInput(parsed_input, size_x, size_y);
		parsed_input[0][0].cost = 0; //First point has no cost ! 
		std::unordered_map<point, point> came_from;
		std::unordered_map<point, int> cost_so_far;
		a_star_search(parsed_input, parsed_input[0][0], parsed_input[parsed_input.size() - 1][parsed_input[0].size() - 1], came_from, cost_so_far);
		std::cout << cost_so_far[parsed_input[parsed_input.size() - 1][parsed_input[0].size() - 1]] << std::endl;
	}
}

