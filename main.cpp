#include <iostream>
#include <string>
#include <filesystem>
using namespace std::filesystem;

void downloadFile(const std::string &url, const std::string& outputName) {
    system(("curl --user-agent \"Mozilla/5.0 (X11; Linux x86_64; rv:60.0) Gecko/20100101 Firefox/81.0\" " + url + " --output " + outputName).c_str());
}

void buildBuildTools(std::string &baseDir, const std::string& version) {
    std::string buildToolsBuildDir = baseDir + "/BuildTools-Build";
    bool shouldBuild = true;
    if (shouldBuild) {
        create_directory(buildToolsBuildDir);
        current_path(buildToolsBuildDir);
        std::cout << "Downloading BuildTools..." << std::endl;
        downloadFile(
                "https://hub.spigotmc.org/jenkins/job/BuildTools/lastSuccessfulBuild/artifact/target/BuildTools.jar", "BuildTools.jar");
        std::cout << "Building Spigot with BuildTools..." << std::endl;
        system(("/usr/lib/jvm/java-16-openjdk/bin/java -jar BuildTools.jar --rev " + version).c_str());
        current_path(buildToolsBuildDir + "/work");
        system(("cp minecraft_server." + version + ".jar ../..").c_str());
        std::cout << "Finished building Spigot with BuildTools..." << std::endl;
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
        system("git clone https://github.com/PaperMC/Paperclip .");
        system("gradle build");
        current_path(executableDir);
        system("cp paperclip-2.0.2-SNAPSHOT.jar ../../..");
        std::cout << "Finished building paperclip!" << std::endl;
    }
}

int main() {
    //TODO Document Requirements: Maven, Gradle, Git, Bsdiff, Java 16

    //TODO build paperclip and buildtools in parallel if possible
    std::cout << "Hello, World!" << std::endl;
    std::string baseDir = current_path().string();
    buildPaperclip(baseDir);
    current_path(baseDir);
    buildBuildTools(baseDir, "1.17.1");
    current_path(baseDir);
    //TODO Parse vanilla server url found in BuildTools-Build/BuildData/info.json
    //1.17.1 vanilla server url is https://launcher.mojang.com/v1/objects/a16d67e5807f57fc4e550299cf20226194497dc2/server.jar
    return 0;
}
