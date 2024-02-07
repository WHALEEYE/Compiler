#include <L3.h>
#include <label_globalizer.h>

using namespace std;

namespace L3 {
void globalizeLabels(Program *P) {
  string longestLabel = "";
  for (auto F : P->getFunctions())
    for (auto &[labelName, label] : F->getLabels())
      if (labelName.length() > longestLabel.length())
        longestLabel = labelName;

  longestLabel += "_global";
  int count = 0;

  for (auto F : P->getFunctions())
    for (auto &[_, label] : F->getLabels()) {
      label->name = longestLabel + to_string(count);
      count++;
    }
}
} // namespace L3