#ifndef APP_MANAGER_HPP
#define APP_MANAGER_HPP

enum class LabSelection {
  NONE,
  LAB1_1,
  LAB1_2,
  LAB2_1,
  LAB3_1,
  LAB4_1,
  LAB4_2,
  LAB5_1,
  LAB5_2,
  LAB6_1,
  LAB6_2
};

constexpr LabSelection DEFAULT_LAB = LabSelection::LAB1_1;

void appManagerSetup();
void appManagerLoop();

LabSelection getCurrentLab();
void setCurrentLab(LabSelection lab);

#endif
