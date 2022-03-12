#ifndef QUBESPKG_H
#define QUBESPKG_H
#include <string>
#include <vector>

class qubesPkg
{
public:

    qubesPkg(std::string projName,std::string pkgName);
    qubesPkg(std::string projName,std::string pkgName, bool ignoreDep);
    qubesPkg(std::string projName,std::string pkgName, bool ignoreDep, bool usePersonalRepo);

    int download();

    int unzip();

    void addBuildFolder(std::string folder);

    int createPackage();

    int installDevPackage();

    void cleanUp();

    int readVersion();

    static void init();

    bool ignoreDependencies{false};
    bool usePersonalRepo{false};
    std::string projectName{};
private:
    std::string packageName{};
    std::string projectUrl{};
    std::string packageVersion{};

    std::vector<std::string> buildFolders{};
    std::vector<std::string> installedPkg{};

    static int runCmd(std::string cmd);
    int installPkg(std::string pkg);
    int removePkg(std::string pkg);

    int removeFolder();

    void getProjectUrl();
};

#endif // QUBESPKG_H
