#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <cpptoml.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <stdio.h>

static void showSettings (bool * p_open);
static void showAccounts (bool * p_open);
static void showAdvanced (bool * p_open);
static void showLedger (bool * p_open);
static void showPeers (bool * p_open);
static void createBlock (bool * p_open);
static void blockViewer (bool * p_open);
static void accountViewer (bool * p_open);
static void nodeStatistics (bool * p_open);

namespace nano_imgui
{
class config
{
public:
	/** Wallet UI related settings*/
	class wallet
	{
	public:
		/** If true, log to stderr in addition to file */
		bool log_to_stderr{ false };
		/** Scale in plank 1x10^30plank==1nano */
		uint64_t scale{ 0 };
	} wallet;

	/** Read the TOML file \p toml_config_path if it exists, and override any settings using \p config_overrides */
	config (std::string const & toml_config_path = "", std::vector<std::string> const & config_overrides = std::vector<std::string> ())
	{
		read_config (toml_config_path, config_overrides);
		if (tree->contains ("wallet"))
		{
			auto wallet_l (tree->get_table ("wallet"));
			wallet.log_to_stderr = wallet_l->get_as<bool> ("log_to_stderr").value_or (wallet.log_to_stderr);
			wallet.scale = wallet_l->get_as<uint64_t>("scale").value_or(wallet.scale);
		}
	}

	/**
	 * Returns a complete TOML config with documented default config values
	 */
	std::string export_documented ()
	{
		auto root_l = cpptoml::make_table ();
		auto wallet_l = cpptoml::make_table ();
		
		root_l->insert ("wallet", wallet_l);
		put (wallet_l, "log_to_stderr", wallet.log_to_stderr, "Log to standard error in addition to file.\ntype:bool");
		put(wallet_l, "scale", wallet.scale, "Scale in plank 1x10^30 plank == 1 nano.\ntype:uint64");

		std::stringstream ss, ss_processed;
		cpptoml::toml_writer writer{ ss, "" };
		root_l->accept (writer);
		std::string line;
		while (std::getline (ss, line, '\n'))
		{
			if (!line.empty () && line[0] != '#' && line[0] != '[')
			{
				line = "#" + line;
			}
			ss_processed << line << std::endl;
		}
		return ss_processed.str ();
	}

	bool config_file_exists (std::string const & toml_config_path) const
	{
		return boost::filesystem::exists (toml_config_path);
	}

private:
	std::shared_ptr<cpptoml::table> tree;

	template <typename T>
	void put (std::shared_ptr<cpptoml::table> table, std::string const & key, T value, std::string const & doc)
	{
		table->insert (key, value);
		table->document (key, doc);
	}

	void read_config (std::string const & toml_config_path, std::vector<std::string> const & config_overrides = std::vector<std::string> ())
	{
		try
		{
			std::stringstream config_overrides_stream;
			for (auto const & entry : config_overrides)
			{
				config_overrides_stream << entry << std::endl;
			}
			config_overrides_stream << std::endl;

			if (!toml_config_path.empty () && boost::filesystem::exists (toml_config_path))
			{
				std::ifstream input (toml_config_path);
				tree = cpptoml::parse_base_and_override_files (config_overrides_stream, input, cpptoml::parser::merge_type::ignore, false);
			}
			else
			{
				std::stringstream stream_empty;
				stream_empty << std::endl;
				tree = cpptoml::parse_base_and_override_files (config_overrides_stream, stream_empty, cpptoml::parser::merge_type::ignore, false);
			}
		}
		catch (std::runtime_error const & err)
		{
			auto parse_err = std::string ("TOML config error: ") + err.what ();
			throw std::runtime_error (parse_err);
		}
	}
};
}