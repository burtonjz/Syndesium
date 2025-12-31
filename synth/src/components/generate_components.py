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
    
def generate_component_include_file():
    componentDir = Path(__file__).parent 
    components = os.listdir(componentDir)
    includes = "\n".join(f'#include "components/{c}"' for c in components if c.endswith(".hpp"))

    templatePath = componentDir / "Components.hpp.in"
    with open(templatePath, "r") as f:
        template = f.read()
    
    output = template.replace("@INCLUDES@", includes)
    
    # Write output
    output_path = componentDir / "Components.hpp"
    with open(output_path, "w") as f:
        f.write(output)

generate_component_include_file()