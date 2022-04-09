#ifndef QUBESPKG_H
#define QUBESPKG_H
#include <string>
#include <vector>
#include <map>

enum class PkgInstallFlag
{
    ALL,
    FOR_DEV,
    FOR_PROD,
    IGNORE
};

class qubesPkg
{
public:

    qubesPkg(std::string projName);
    qubesPkg(std::string projName, bool ignoreDep);
    qubesPkg(std::string projName, bool ignoreDep, bool usePersonalRepo);

    int download();

    int unzip();

    void addPackageName(std::string pkgName,PkgInstallFlag installForBuildProc);

    int createPackage();

    int installPackages(bool all=false);

    void cleanUp();

    int readVersion();

    static void init();

    bool ignoreDependencies{false};
    bool usePersonalRepo{false};
    std::string projectName{};
private:
    std::string projectUrl{};
    std::string packageVersion{};

    std::map<std::string,PkgInstallFlag> packages{};
    std::vector<std::string> installedPkg{};

    int installPkg(std::string pkg);
    static int installPkgAPT(std::string pkg);
    int removePkg(std::string pkg);

    static int isPackageInstalled(std::string p);

    int removeFolder();

    void getProjectUrl();

    static int runCmd(std::string cmd);
};

#endif // QUBESPKG_H
