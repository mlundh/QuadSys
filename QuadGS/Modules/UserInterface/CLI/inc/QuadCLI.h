/*
 * QuadCLI.h
 *
 *  Created on: March, 2015
 *      Author: martin
 */

#ifndef _QUADCLI_H_
#define _QUADCLI_H_
#include <string>

namespace QuadGS {
namespace QuadCLI{

    bool ExecuteNextCommand();
    void SetPrompt(std::string prompt);
    void InitCLI();


}
} /* namespace QuadGS */

#endif /* _QUADCLI_H_ */
