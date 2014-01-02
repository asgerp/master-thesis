//
//  firecommand.h
//  CommandInterface
//
//  Created by Asger Pedersen on 22/12/13.
//  Copyright (c) 2013 Asger Pedersen. All rights reserved.
//

#ifndef __CommandInterface__firecommand__
#define __CommandInterface__firecommand__

#include <iostream>
#include "json_spirit.h"
#include <map>

class Firecommand {
    json_spirit::mValue commands;
public:
    Firecommand(std::string);
    //~Firecommand();
    void fire_command (std::string);
    void store_command(std::string, std::string);
    void delete_command(std::string);
private:
    void load_commands(std::string);
    const json_spirit::mValue& find_value(const json_spirit::mObject& obj, const std::string& name );
    
};

#endif /* defined(__CommandInterface__firecommand__) */
