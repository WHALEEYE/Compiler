#include <L3.h>
#include <label_globalizer.h>

#include <utility>

using namespace std;

namespace L3 {

void renameLabel(Label *label, string newName) { label->name = std::move(newName); }

string LabelGlobalizer::prefix = ":global";
int LabelGlobalizer::count = 0;
bool LabelGlobalizer::initialized = false;

void LabelGlobalizer::initialize(Program *P) {
  if (initialized)
    return;

  for (auto F : P->getFunctions())
    for (auto &[labelName, _] : F->getLabels())
      if (labelName.length() > LabelGlobalizer::prefix.length())
        LabelGlobalizer::prefix = labelName;

  LabelGlobalizer::prefix += "_global";
  LabelGlobalizer::count = 0;
  LabelGlobalizer::initialized = true;
}

void globalizeLabels(Program *P) {
  LabelGlobalizer::initialize(P);

  for (auto F : P->getFunctions())
    for (auto &[_, label] : F->getLabels())
      renameLabel(label, LabelGlobalizer::generateNewName());
}

} // namespace L3