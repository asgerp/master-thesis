//
//  firecommand.cpp
//  CommandInterface
//
//  Created by Asger Pedersen on 22/12/13.
//  Copyright (c) 2013 Asger Pedersen. All rights reserved.
//

#include "firecommand.h"
#include <boost/filesystem.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <json/json.h>
#include <map>

class FireCommand {
    std::map<std::string, std::string> commands;
public:
    FireCommand(std::string);
    ~FireCommand();
    void fire_command (std::string);
    void store_command(std::string, std::string);
    void delete_command(std::string);
private:
    void load_commands(std::string);
    
};



FireCommand::FireCommand(std::string path_to_commands){
    load_commands(path_to_commands);
}

void FireCommand::fire_command (std::string template_key){
    std::string current_command = commands.find(template_key)->second;
    std::system(current_command.c_str());
}

void FireCommand::load_commands(std::string path_to_commands) {
    
}