module;
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>

export module resource;

export template<typename resource_type>
class resource {
public:
	/// needed as constructor can't call child's overridden function
	void load(const std::string_view ext) {
		// recursively load all files in the resources folder
		for (
			const auto path :
			std::filesystem::recursive_directory_iterator("res")
		) {
			if (path.path().extension() != ext) {
				continue;
			}
			const auto name = path.path().stem().string();
			auto value = load_from_file(path.path());
			resources.insert({ name, value });
		}
	}
protected:
	virtual resource_type load_from_file(
		const std::filesystem::path& path
	) const = 0;

	std::unordered_map<std::string, resource_type> resources{};
};
