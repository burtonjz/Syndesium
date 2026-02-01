"""
 * Copyright (C) 2025 Jared Burton
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
"""

import os
import re
from pathlib import Path

def extract_component_types(componentFile):
    with open(componentFile, "r") as f:
        content = f.read()

    # Find the COMPONENT_TYPE_LIST macro definition
    pattern = r'#define\s+COMPONENT_TYPE_LIST\s+((?:.*?\\\n)*.*?)(?:\n(?!.*\\)|$)'
    matches = re.search(pattern, content, re.MULTILINE)
        
    if not matches:
        raise ValueError("Could not find 'enum class ComponentType' in file")
    
    body = matches.group(1)
    components = re.findall(r'X\((\w+)\)', body)

    return [ c for c in components ]

    
def generate_component_config():
    configDir = Path(__file__).parent 
    componentFile = Path(__file__).parent.parent.parent.parent / "shared" / "types" / "ComponentType.hpp"    
    components = extract_component_types(componentFile)
  
    includes = "\n".join(f'#include "configs/{name}Config.hpp"' for name in components)

    templatePath = configDir / "ComponentConfig.hpp.in"
    with open(templatePath, "r") as f:
        template = f.read()
    
    output = template.replace("@INCLUDES@", includes)
    
    # Write output
    output_path = configDir / "ComponentConfig.hpp"
    with open(output_path, "w") as f:
        f.write(output)

generate_component_config()