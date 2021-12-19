#include <filesystem>
#include <fstream>
#include <iostream>
#include <bitset>
#include <unordered_map>
#include <utility>

namespace fs = std::filesystem;

std::unordered_map<char, std::string> hex_2_dec 
{
	{'0', "0000"},
	{'1', "0001"},
	{'2', "0010"},
	{'3', "0011"},
	{'4', "0100"},
	{'5', "0101"},
	{'6', "0110"},
	{'7', "0111"},
	{'8', "1000"},
	{'9', "1001"},
	{'A', "1010"},
	{'B', "1011"},
	{'C', "1100"},
	{'D', "1101"},
	{'E', "1110"},
	{'F', "1111"}
};

//something wrong with this function :(
//unsigned long long int bin_2_dec(const std::string& final_binary)
//{
//	unsigned long long int decimal_value = 0;
//	const auto size = static_cast<int>(final_binary.size());
//	for (int i = size - 1; i >= 0; i--)
//	{
//		const unsigned long long mul = 1 << (size - i - 1);
//		const unsigned long long int bit_value = static_cast<unsigned long long int>(final_binary[i]) - '0';
//		decimal_value += bit_value * mul;
//	}
//	return decimal_value;
//}


unsigned long long int bin_2_dec(const std::string& str)
{
	const std::string& n = str;
	unsigned long long int val = 0;
	unsigned long long int temp = 1;
	const long long int len = n.length();
	for (long long int i = len - 1; i >= 0; i--) 
	{
		if (n[i] == '1')
		{
			val += temp;
		}
		temp = temp * 2;
	}
	return val;
}

struct packet
{
	packet(const int version_, const int type_ID_)
		:version(version_), type_ID(type_ID_)
	{
	}

	packet(const int version_, const int type_ID_, std::vector<std::unique_ptr<packet>> subpackets)
		:version(version_), type_ID(type_ID_), subpackets(std::move(subpackets))
	{
	}

	virtual ~packet() = default;

	int version;
	int type_ID;
	std::vector<std::unique_ptr<packet>> subpackets;

	virtual void print(int level)
	{
		std::cout << "LEVEL " << level << " --- Packet version:" << version << " type_id:" << std::endl;
	}

};

struct packet_literal final : packet
{
	packet_literal(const int version_, const int type_ID_, const unsigned long long int value_)
		: packet(version_, type_ID_), value(value_)
	{
	}

	void print(int level) override
	{
		std::cout << "LEVEL " << level << " --- Literal packet version:" << version << " type_id:"  << type_ID << " value:" << value << std::endl;
	}

	unsigned long long int value;
};

struct packet_operator_11_bits final : packet
{
	packet_operator_11_bits(const int version_, const int type_ID_, const int length_type_ID_, const int number_of_subpackets_, std::vector<std::unique_ptr<packet>> subpackets_)
		: packet(version_, type_ID_, std::move(subpackets_)),
		  length_type_ID(length_type_ID_), number_of_subpackets(number_of_subpackets_)
	{
	}

	void print(int level) override
	{
		std::cout << "LEVEL " << level << " --- Operator packet 11 bits version:" << version << " type_id:"  << type_ID << " length_type_ID:" << length_type_ID << " number_of_subpackets:" << number_of_subpackets << std::endl;
	}

	int length_type_ID;
	int number_of_subpackets;
};

struct packet_operator_15_bits final : packet
{
	packet_operator_15_bits(const int version_, const int type_ID_, const int length_type_ID_, const int bits_in_subpackets_, std::vector<std::unique_ptr<packet>> subpackets_)
		: packet(version_, type_ID_, std::move(subpackets_)),
		  length_type_ID(length_type_ID_), bits_in_subpackets(bits_in_subpackets_)
	{
	}

	void print(int level) override
	{
		std::cout << "LEVEL " << level << " --- Operator packet 15 bits version:" << version << " type_id:"  << type_ID << " length_type_ID:" << length_type_ID << " bits_in_subpackets: " << bits_in_subpackets << std::endl;
	}

	int length_type_ID;
	int bits_in_subpackets;
};



std::string from_hex_to_bin(const std::string& hex_string)
{
	std::string results{};
	for (auto character : hex_string)
	{
		results += hex_2_dec[character];
	}
	return results;
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
		const auto sz = file_size(path);
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

std::unique_ptr<packet> decode_packet(std::string& data, int level)
{
	std::string copy_data = data;
	const std::string version_str = copy_data.substr(0, 3);
	const std::string type_id_str = copy_data.substr(3, 3);
	copy_data = copy_data.substr(6);
	const auto type_id = bin_2_dec(type_id_str);
	const auto version = bin_2_dec(version_str);
	if(type_id == 4) // literal
	{
		std::string value_str{};
		while (true)
		{
			const auto control_bit = copy_data[0];
			value_str += copy_data.substr(1,4);
			copy_data = copy_data.substr(5);
			if(control_bit == '0')
			{
				break;
			}
		}

		const auto value = bin_2_dec(value_str);
		data = copy_data;
		return std::make_unique<packet_literal>(version, type_id, value);
	}
	// operators

	if (const auto length_type_bit = copy_data[0];
		length_type_bit == '1') // 11 bits operator
	{
		const auto number_of_subpackets_str = copy_data.substr(1, 11);
		copy_data = copy_data.substr(12);
		const auto number_of_subpackets = bin_2_dec(number_of_subpackets_str);

		std::vector<std::unique_ptr<packet>> subpackets;
		for (int i = 0; i < number_of_subpackets; i++)
		{
			//std::string copy_data = data.substr(0, 11);
			subpackets.push_back(decode_packet(copy_data, level + 1));
			//data = data.substr(11);
			subpackets.back()->print(level + 1);
		}
		data = copy_data;
		return std::make_unique<packet_operator_11_bits>(version, type_id, 1, number_of_subpackets, std::move(subpackets));
	}
	// 15 bits operator
	const auto bits_in_subpackets_str = copy_data.substr(1, 15);
	copy_data = copy_data.substr(16);
	const auto bits_in_subpackets = bin_2_dec(bits_in_subpackets_str);

	std::vector<std::unique_ptr<packet>> subpackets;
	std::string subdata = copy_data.substr(0, bits_in_subpackets);
	while(!subdata.empty())
	{
		subpackets.push_back(decode_packet(subdata, level + 1));
		subpackets.back()->print(level + 1);
	}
	copy_data = copy_data.substr(bits_in_subpackets);

	data = copy_data;
	return std::make_unique<packet_operator_15_bits>(version, type_id, 0, bits_in_subpackets, std::move(subpackets));
}



std::vector<std::unique_ptr<packet>> decode_message(const std::string& raw_data)
{
	std::string copy = raw_data;
	std::vector<std::unique_ptr<packet>> packets;
	while (!copy.empty())
	{
		packets.push_back(decode_packet(copy, 0));
		packets.back()->print(0);
		if (copy.find_first_not_of('0') == std::string::npos)
		{
			break;
		}
	}

	return packets;
}

unsigned long long int decode_packet_and_calculate(std::string& data, int level)
{
	const std::string version_str = data.substr(0, 3);
	const std::string type_id_str = data.substr(3, 3);
	data = data.substr(6);
	const auto type_id = bin_2_dec(type_id_str);
	const auto version = bin_2_dec(version_str);
	if(type_id == 4) // literal
	{
		std::string value_str{};
		while (true)
		{
			const auto control_bit = data[0];
			value_str += data.substr(1,4);
			data = data.substr(5);
			if(control_bit == '0')
			{
				break;
			}
		}

		const auto value = bin_2_dec(value_str);
		std::cout << "LEVEL " << level << " --- value=" << value << std::endl;
		return value;
	}
	// operators
	std::vector<unsigned long long int> subpackets_values;
	if (const auto length_type_bit = data[0];
		length_type_bit == '1') // 11 bits operator
	{
		const auto number_of_subpackets_str = data.substr(1, 11);
		data = data.substr(12);
		const auto number_of_subpackets = bin_2_dec(number_of_subpackets_str);

		for (int i = 0; i < number_of_subpackets; i++)
		{
			subpackets_values.push_back(decode_packet_and_calculate(data, level + 1));
			std::cout << "LEVEL " << level + 1 << " --- value=" << subpackets_values.back() << std::endl;
		}
	}
	else
	{
		// 15 bits operator
		const auto bits_in_subpackets_str = data.substr(1, 15);
		data = data.substr(16);
		const auto bits_in_subpackets = bin_2_dec(bits_in_subpackets_str);

		std::string subdata = data.substr(0, bits_in_subpackets);
		while(!subdata.empty())
		{
			subpackets_values.push_back(decode_packet_and_calculate(subdata, level + 1));
			std::cout << "LEVEL " << level + 1 << " --- value=" << subpackets_values.back() << std::endl;
		}
		data = data.substr(bits_in_subpackets);
	}

	switch (type_id)
	{
	case 0:
		{
			unsigned long long int sum = 0;
			for (const auto& subpacket_value : subpackets_values)
			{
				sum += subpacket_value;
			}
			std::cout << "LEVEL " << level << " --- sum=" << sum << std::endl;
			return sum;
		}
	case 1:
		{
			unsigned long long int product = 1;
			for (const auto& subpacket_value : subpackets_values)
			{
				product *= subpacket_value;
			}
			std::cout<< "LEVEL " << level << " --- product=" << product << std::endl;
			return product;
		}
	case 2:
		{
			unsigned long long int min_value = std::numeric_limits<int>::max();
			for (const auto& subpacket_value : subpackets_values)
			{
				min_value = std::min(min_value, subpacket_value);
			}
			std::cout << "LEVEL " << level << " --- min=" << min_value << std::endl;
			return min_value;
		}
	case 3:
		{
			unsigned long long int max_value = 0;
			for (const auto& subpacket_value : subpackets_values)
			{
				max_value = std::max(max_value, subpacket_value);
			}
			std::cout << "LEVEL " << level << " --- max=" << max_value << std::endl;
			return max_value;
		}
	case 5:
		{
			std::cout << "LEVEL " << level << " --- greater=" << (subpackets_values[0] > subpackets_values[1] ? 1 : 0) << std::endl;
			return subpackets_values[0] > subpackets_values[1] ? 1 : 0;
		}
	case 6:
		{
			std::cout << "LEVEL " << level << " --- less=" << (subpackets_values[0] < subpackets_values[1] ? 1 : 0) << std::endl;
			return subpackets_values[0] < subpackets_values[1] ? 1 : 0;
		}
	case 7:
		{
			std::cout << "LEVEL " << level << " --- equal=" << (subpackets_values[0] == subpackets_values[1] ? 1 : 0) << std::endl;
			return subpackets_values[0] == subpackets_values[1] ? 1 : 0;
		}
		default:
		std::cout << "wrong case" << std::endl;
		return 0;
	}
}

unsigned long long int decode_message_and_calculate(const std::string& raw_data)
{
	std::string copy = raw_data;
	unsigned long long int result = 0;
	while (!copy.empty())
	{
		std::cout << "LEVEL 0 --- result=" << result << std::endl;
		result += decode_packet_and_calculate(copy, 0);

		if (copy.find_first_not_of('0') == std::string::npos)
		{
			break;
		}
	}

	return result;
}

//int calculate(const std::vector<std::unique_ptr<packet>>& packets)
//{
//	for (const auto& packet : packets)
//	{
//		if (packet->type_ID)
//	}
//}

void print_packets(const std::vector<std::unique_ptr<packet>>& packets, int level = 0)
{
	for (const auto& packet : packets)
	{
		for(int i = 0; i < level; i++)
		{
			std::cout << '\t';
		}
		packet->print(level);
		if(packet->type_ID != 4)
		{
			print_packets(packet->subpackets, level + 1);
		}
	}
}

int main()
{
	{
		// part 01
		// auto input = read_file("input_day16_op_11b.txt");
		// auto input = read_file("input_day16_op_15b.txt");
		// auto input = read_file("input_day16_literal.txt");
		auto input = read_file("input_day16.txt");

		input = trim(input);
		const auto binary_input = from_hex_to_bin(input);
		const auto packets = decode_message(binary_input);

		auto sum_of_version_ids = 0;

		for (const auto& packet : packets)
		{
			sum_of_version_ids += packet->version;
		}
		std::cout << sum_of_version_ids << std::endl;
		print_packets(packets);
	}

	{
		// part02
		auto input = read_file("input_day16.txt");
		input = trim(input);
		const auto binary_input = from_hex_to_bin(input);
		const auto result = decode_message_and_calculate(binary_input);

		std::cout << result << std::endl;
	}
}