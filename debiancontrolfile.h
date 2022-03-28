#ifndef DEBIANCONTROLFILE_H
#define DEBIANCONTROLFILE_H

#include <string>
#include <vector>

class debianControlFile
{
public:
    debianControlFile(std::string file);

    std::vector<std::string> getBuildDependencies();
private:
    void readFile();

    std::vector<std::string> buildDep{};
    std::string fileName;
};

#endif // DEBIANCONTROLFILE_H
