/**************************************************************************/
/*                                                                        */
/*                          WWIV Version 5.x                              */
/*             Copyright (C)2015-2017, WWIV Software Services             */
/*                                                                        */
/*    Licensed  under the  Apache License, Version  2.0 (the "License");  */
/*    you may not use this  file  except in compliance with the License.  */
/*    You may obtain a copy of the License at                             */
/*                                                                        */
/*                http://www.apache.org/licenses/LICENSE-2.0              */
/*                                                                        */
/*    Unless  required  by  applicable  law  or agreed to  in  writing,   */
/*    software  distributed  under  the  License  is  distributed on an   */
/*    "AS IS"  BASIS, WITHOUT  WARRANTIES  OR  CONDITIONS OF ANY  KIND,   */
/*    either  express  or implied.  See  the  License for  the specific   */
/*    language governing permissions and limitations under the License.   */
/**************************************************************************/
#include "wwivutil/config/config.h"

#include "core/command_line.h"
#include "core/datafile.h"
#include "core/file.h"
#include "core/strings.h"
#include "sdk/config.h"
#include "sdk/filenames.h"
#include "sdk/net.h"
#include "sdk/networks.h"
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using std::cerr;
using std::cout;
using std::endl;
using std::make_unique;
using std::setw;
using std::string;
using std::unique_ptr;
using std::vector;
using wwiv::core::BooleanCommandLineArgument;
using namespace wwiv::core;
using namespace wwiv::sdk;
using namespace wwiv::strings;

namespace wwiv {
namespace wwivutil {

static int show_version(const Config& config) {
  cout << "5.2 Versioned Config    : " << std::boolalpha << config.versioned_config_dat()
       << std::endl;
  cout << "Written By WWIV Version : " << config.written_by_wwiv_num_version() << std::endl;
  cout << "Config Revision #       : " << config.config_revision_number() << std::endl;

  return 0;
}

void save_config(configrec& c) {
  DataFile<configrec> file(CONFIG_DAT,
                           File::modeBinary | File::modeReadWrite | File::modeCreateFile);
  if (file) {
    file.Write(&c);
  }
}

static int set_version(const Config& config, int wwiv_ver, int revision) {
  if (!config.versioned_config_dat()) {
    cout << "Can only set the wwiv_ersion and config revision on a 5.1 or higher versioned "
            "config.dat"
         << std::endl;
    return 1;
  }

  configrec cfg430 = *config.config();
  cout << cfg430.systemname << std::endl;
  cout << " set_version: wwiv_ver: " << wwiv_ver << " set_version: " << revision << std::endl;
  auto& h = cfg430.header.header;
  if (wwiv_ver >= 500) {
    cout << "setting wwiv_ver to " << wwiv_ver << std::endl;
    h.written_by_wwiv_num_version = wwiv_ver;
  }
  if (revision > 0) {
    cout << "setting revision to " << revision << std::endl;
    h.config_revision_number = revision;
  }
  h.config_size = sizeof(configrec);
  memset(&h.unused, 0, sizeof(h.unused));
  h.padding[0] = 0;

  cout << "Wrote Config.dat" << std::endl;
  save_config(cfg430);
  return 0;
}

class ConfigVersionCommand : public UtilCommand {
public:
  ConfigVersionCommand() : UtilCommand("version", "Sets or Gets the config version") {}
  std::string GetUsage() const override final {
    std::ostringstream ss;
    ss << "Usage: " << std::endl << std::endl;
    ss << "  get : Gets the config.dat version information." << std::endl;
    ss << "  set : Sets the config.dat version information." << std::endl << std::endl;
    return ss.str();
  }
  int Execute() override final {
    if (remaining().empty()) {
      std::cout << GetUsage() << GetHelp() << endl;
      return 2;
    }
    string set_or_get(remaining().front());
    StringLowerCase(&set_or_get);

    if (set_or_get == "get") {
      return show_version(*this->config()->config());
    } else if (set_or_get == "set") {
      return set_version(*this->config()->config(), iarg("wwiv_version"), iarg("revision"));
    }
    return 1;
  }
  bool AddSubCommands() override final {
    add_argument({"wwiv_version", "WWIV Version that created this config.dat", ""});
    add_argument({"revision", "Configuration revision number", ""});
    return true;
  }
};

bool ConfigCommand::AddSubCommands() {
  add(make_unique<ConfigVersionCommand>());
  return true;
}

} // namespace wwivutil
} // namespace wwiv
