#include<iostream>
#include "mainMenu.hpp"
int main(int argc, char *argv[])
{
    Arcade *mainMenu = new MainMenu;
    mainMenu->run();
    delete mainMenu;
    return 0;
}