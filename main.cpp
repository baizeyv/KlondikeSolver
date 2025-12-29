#include <iostream>

#include "src/poker.h"
#include "src/state.h"

int main() {
  poker p("W#wg#diA#Kxof#RYItC#vhFQSk#MTuOmNn#XpZJyBlLEsaDzcbjGrUqHVPe");
  state st(&p);
  std::cout << st.to_string() << std::endl;
  st.move_card(0, 1, 2);
  std::cout << st.to_string() << std::endl << st.to_serialized() << std::endl;
  system("pause");
  return 0;
}
