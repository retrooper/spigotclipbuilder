#include <iostream>
#include <string>
#include <filesystem>
using namespace std::filesystem;

void execute_command(const std::string& command) {
    std::system(command.c_str());
}

void downloadFile(const std::string &url, const std::string& outputName) {
        execute_command("curl --user-agent \"Mozilla/5.0 (X11; Linux x86_64; rv:60.0) Gecko/20100101 Firefox/81.0\" " + url + " --output " + outputName);
}

void buildBuildTools(std::string javaBinary, std::string &baseDir, const std::string& version) {
    std::string buildToolsBuildDir = baseDir + "/BuildTools-Build";
    bool shouldBuild = true;
    if (shouldBuild) {
        create_directory(buildToolsBuildDir);
        current_path(buildToolsBuildDir);
        std::cout << "Downloading BuildTools..." << std::endl;
        downloadFile(
                "https://hub.spigotmc.org/jenkins/job/BuildTools/lastSuccessfulBuild/artifact/target/BuildTools.jar", "BuildTools.jar");
        std::cout << "Building Spigot with BuildTools..." << std::endl;
        execute_command(javaBinary + " -jar BuildTools.jar --rev " + version);
        std::string workPath = buildToolsBuildDir + "/work";
        if (exists(workPath)) {
            current_path(workPath);
            execute_command("cp minecraft_server." + version + ".jar ../..");
            std::cout << "Finished building Spigot with BuildTools..." << std::endl;
        }
        else {
            std::cerr << "Failed to build Spigot with BuildTools, are you sure you have all required components installed?" << std::endl;
            std::exit(-1);
        }
    }
}

void buildPaperclip(std::string &baseDir) {
    std::string paperClipBuildDir = baseDir + "/Paperclip-Build";
    std::string executableDir = paperClipBuildDir + "/build/libs";
    bool shouldBuild = !exists(executableDir);
    if (shouldBuild) {
        std::cout << "Building PaperClip..." << std::endl;
        create_directory(paperClipBuildDir);
        current_path(paperClipBuildDir);
        execute_command("git clone https://github.com/PaperMC/Paperclip .");
        execute_command("gradle build");
        if (exists(executableDir)) {
            current_path(executableDir);
            //Copy the first file in the directory (there should only be one)
            for (const auto &entry: directory_iterator(executableDir)) {
                execute_command("cp " + entry.path().string() + " ../../..");
                break;
            }
            std::cout << "Finished building paperclip!" << std::endl;
        }
        else {
            std::cerr << "Failed to build PaperClip. Are you sure you have all required components installed?" << std::endl;
            std::exit(-1);
        }
    }
}

void build(const std::string& version, const std::string& javaBinary) {
    std::cout << "Starting to build a " << version << " SpigotClip build!" << std::endl;
    std::cout << "This Java Binary will be used to build PaperClip and Spigot: " << javaBinary << std::endl;
    std::string baseDir = current_path().string();
    buildPaperclip(baseDir);
    current_path(baseDir);
    buildBuildTools(javaBinary, baseDir, version);
    current_path(baseDir);
}

int main() {
    //TODO Document Requirements: Maven, Gradle, Git, Bsdiff, Java 16

    //TODO build paperclip and buildtools in parallel if possible

    //TODO Parse vanilla server url found in BuildTools-Build/BuildData/info.json
    build("1.17.1", "/usr/lib/jvm/java-16-openjdk/bin/java");
    return 0;
}
