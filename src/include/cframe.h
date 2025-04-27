/* cframe - v1.3.1 - MIT License - https://github.com/zb1ndev/zansi.h 

    MIT License
    Copyright (c) 2025 Joel Zbinden

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

    WARNING : Depends on https://github.com/zb1ndev/zstring.h | Minimum Version : 1.3.0

    Version 1.1.0 Change-Log : 
    - Added Unix Support

    Version 1.2.0 Change-Log :
    - Made Connection handling better
    - Reorganized the project

    Version 1.3.0 Change-Log : 
    - Added Node Instance Management Functions ( unix and windows )

    Version 1.3.1 Change-Log :
    - Reorganized the project
    - Updated 'zstring.h'

*/
#if !defined(CFRAME_H)
#define CFRAME_H
    
    #include "./zstring.h" // zstring.h : Version 1.3.0

    #include "./common.h"
    #include "./http.h"
    #include "./handler.h"
    #include "./node.h"

#endif // CFRAME_H