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
#include <fstream>
//#include <json/json.h>
#include "json_spirit.h"
#include <map>


using namespace json_spirit;

Firecommand::Firecommand(std::string path_to_commands){
    load_commands(path_to_commands);
}

void Firecommand::fire_command (std::string template_key){

    std::string current_command = find_value( commands.get_obj(), template_key ).get_str();
    std::system(current_command.c_str());
}

void Firecommand::load_commands(std::string path_to_commands) {
    
    std::ifstream is( path_to_commands.c_str() );
    
    mValue value;
    read( is, value );
    std::string content( (std::istreambuf_iterator<char>(is) ),
                        (std::istreambuf_iterator<char>()    ) );
    
    commands = value;
}

const mValue& Firecommand::find_value(const mObject& obj, const std::string& name )
{
    mObject::const_iterator i = obj.find(name);

    assert(i != obj.end());
    assert(i->first == name);
    
    return i->second;
}


