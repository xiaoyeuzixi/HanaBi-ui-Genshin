#include "ui/shell.h"
namespace hb {
void Shell::DrawCharacterUi(int character,CharacterUiPart part,const CharacterUiArgs& args) {
    switch(character) {
    case 0: DrawGanyuUi(part,args); break;
    case 3: DrawYaeUi(part,args); break;
    case 4: DrawYoimiyaUi(part,args); break;
    case 5: DrawNahidaUi(part,args); break;
    case 6: DrawFurinaUi(part,args); break;
    case 7: DrawFaruzanUi(part,args); break;
    case 8: DrawZhongliUi(part,args); break;
    default: DrawGanyuUi(part,args); break;
    }
}
}
