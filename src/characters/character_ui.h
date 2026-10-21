#pragma once
#include "ui/ui_ctx.h"
namespace hb {
enum class CharacterUiPart { Chrome,WindowControl,Nav,Sidebar,Card };
struct CharacterUiArgs {
    Rc box;
    float hover=0;
    int index=0;
    float x=0,y=0,width=0,alpha=1;
};
}
