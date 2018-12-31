#include "../libs/hb-appstore/gui/ListElement.hpp"
#include <string>

char* my_realpath(const char *path, char resolved_path[]);

class FileBrowser : public ListElement
{
  public:
  FileBrowser(const char* pwd);
  bool process(InputEvents* event);
  void render(Element* parent);
  void listfiles();
  void update_path(const char* path);

  std::string* pwd = NULL;
};