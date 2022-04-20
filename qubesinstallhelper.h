#ifndef QUBESINSTALLHELPER_H
#define QUBESINSTALLHELPER_H
#include <vector>
#include "qubespkg.h"

class qubesInstallHelper
{
public:
    qubesInstallHelper(std::vector<qubesPkg*> packages,std::string folder="output");

    bool debug{false};

    void install();

    void writeModuleLoadConf();
private:
    std::vector<qubesPkg*> packages;
    std::string folder{"output"};

    qubesPkg* getProjPackage(std::string name);

    int installMissing();

    static int runCmd(std::string cmd);

};

#endif // QUBESINSTALLHELPER_H
