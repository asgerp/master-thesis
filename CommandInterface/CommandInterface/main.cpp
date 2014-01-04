//
//  main.cpp
//  CommandInterface
//
//  Created by Asger Pedersen on 22/12/13.
//  Copyright (c) 2013 Asger Pedersen. All rights reserved.
//

#include <iostream>
#include "firecommand.h"

int main(int argc, const char * argv[])
{
    
    Firecommand fc(argv[1]);
    std::string command = argv[2];
    fc.fire_command(command);
    return 0;
}