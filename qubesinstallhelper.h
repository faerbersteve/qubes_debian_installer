#ifndef QUBESINSTALLHELPER_H
#define QUBESINSTALLHELPER_H
#include <vector>
#include "qubespkg.h"

class qubesInstallHelper
{
public:
    qubesInstallHelper(std::vector<qubesPkg*> packages,std::string folder="output",bool installQubesManagerOnly=false);

    bool debug{false};

    bool install();

private:
    bool installQubesManagerOnly{false};
    std::string folder{"output"};
    std::vector<qubesPkg*> packages;

    qubesPkg* getProjPackage(std::string name);

    int installMissing();

    void writeModuleLoadConf();

    static int runCmd(std::string cmd);

};

#endif // QUBESINSTALLHELPER_H
