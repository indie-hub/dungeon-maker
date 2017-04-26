 #include "map.h"

 int main(int argc, char** argv)
 {
   Map DungeonMap(60, 40, 0.45);

   if(!DungeonMap.GenerateMap())
   {
     std::cerr << "There was an error generating map" << std::endl;
   }

   DungeonMap.MakeCaverns();

   std::cout << std::endl << DungeonMap << std::endl;

   return 0;
 }
