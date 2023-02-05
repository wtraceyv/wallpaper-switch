#include <cstdlib>
#include <cstring>
#include <stdio.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <ftxui/component/component_options.hpp>
#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"  // for Button, Horizontal, Renderer
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for separator, gauge, text, Element, operator|, vbox, border

using namespace std;
using namespace ftxui;

const string wallpaper_dir = "/home/walter/.wallpapers";
const string file_to_mod = "/home/walter/.config/awesome/zenburn/theme.lua";
const string menu_list_command = "ls " + wallpaper_dir;

// forward declares
void SwitchWallpaper(string filename);

int main() {
  // run command to list files in dir
  FILE *pipe = popen(menu_list_command.c_str(), "r");
  if (!pipe) {
    cout << "Something fucked up with popen." << endl;
    return 1;
  }

  // save each file name by reading from std out of popen FILE*
  // into a vector<string>
  vector<string> wallpaper_file_names;
  char buffer[100];
  while (fgets(buffer, 100, pipe)) {
    wallpaper_file_names.push_back(buffer);
  }
  pclose(pipe);

  // set up ftxui menu to display/choose wallpaper file
  int selected = 0;
  auto screen = ScreenInteractive::TerminalOutput();

  // handle selection
  MenuOption option;
  option.on_enter = [&]{
    string selected_file_name = wallpaper_file_names[selected].substr(0, wallpaper_file_names[selected].length() - 1);
    SwitchWallpaper(selected_file_name);
    screen.ExitLoopClosure()();   
  };

  auto menu = Menu(&wallpaper_file_names, &selected, &option);
  screen.Loop(menu);

  return 0;
}

void SwitchWallpaper(string filename) {
  // when something is selected, I have its index in "selected"
  fstream theme_file(file_to_mod);
  ofstream temp_file("temp.txt");

  // important change
  temp_file << "local theme = {}" << endl;
  temp_file << "theme.wallpaper = \'~/.wallpapers/" << filename << "\'" << endl;

  // insert rest of original theme file (skipping first two lines)
  string line;
  theme_file.ignore(100, '\n');
  theme_file.ignore(100, '\n');
  while(getline(theme_file, line)) {
    temp_file << line << endl;
  }

  // "rename" temp into desired file path
  filesystem::rename("temp.txt", file_to_mod);

  temp_file.close();
  theme_file.close();
}
