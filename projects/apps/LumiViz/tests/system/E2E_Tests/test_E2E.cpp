// ==============================================================================
// test_E2E.cpp – End-to-End System Tests (GoogleTest)
// ==============================================================================
//
// Test:        System_EndToEndTests
// Framework:   GoogleTest
// Type:        system
// Version:     1.0.0
// Date:        2025-12-12
//
// Description:
//   End-to-end system tests that verify complete workflows.
//   Tests the system as a whole, not individual components.
//
// Note:
//   These tests are slow and should run in serial.
//
// ==============================================================================

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

// ==============================================================================
// Mock System Components
// ==============================================================================

class MockFileSystem {
public:
    bool createDirectory(const std::string& path) {
        directories_.push_back(path);
        return true;
    }
    
    bool writeFile(const std::string& path, const std::string& content) {
        files_[path] = content;
        return true;
    }
    
    std::string readFile(const std::string& path) {
        auto it = files_.find(path);
        return (it != files_.end()) ? it->second : "";
    }
    
    bool exists(const std::string& path) {
        return files_.count(path) > 0 || 
               std::find(directories_.begin(), directories_.end(), path) != directories_.end();
    }
    
    void clear() {
        files_.clear();
        directories_.clear();
    }
    
private:
    std::map<std::string, std::string> files_;
    std::vector<std::string> directories_;
};

class MockConfigSystem {
public:
    void set(const std::string& key, const std::string& value) {
        config_[key] = value;
    }
    
    std::string get(const std::string& key, const std::string& defaultVal = "") {
        auto it = config_.find(key);
        return (it != config_.end()) ? it->second : defaultVal;
    }
    
    bool load(MockFileSystem& fs, const std::string& path) {
        std::string content = fs.readFile(path);
        if (content.empty()) return false;
        
        // Simple key=value parsing (line by line)
        std::istringstream stream(content);
        std::string line;
        while (std::getline(stream, line)) {
            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                config_[key] = value;
            }
        }
        
        loaded_ = !content.empty();
        return loaded_;
    }
    
    bool isLoaded() const { return loaded_; }
    
private:
    std::map<std::string, std::string> config_;
    bool loaded_ = false;
};

class MockApplication {
public:
    enum class State { Uninitialized, Initializing, Running, ShuttingDown, Stopped };
    
    bool initialize(MockFileSystem& fs, MockConfigSystem& config) {
        state_ = State::Initializing;
        
        // Create required directories
        fs.createDirectory("data");
        fs.createDirectory("logs");
        fs.createDirectory("cache");
        
        // Load config
        if (!config.isLoaded()) {
            fs.writeFile("config.ini", "app.name=TestApp\napp.version=1.0");
            config.load(fs, "config.ini");
        }
        
        state_ = State::Running;
        return true;
    }
    
    void run() {
        // Simulate work (no actual delay to avoid test hanging)
        workCounter_++;
    }
    
    int getWorkCount() const { return workCounter_; }
    
    bool shutdown() {
        state_ = State::ShuttingDown;
        // Cleanup
        state_ = State::Stopped;
        return true;
    }
    
    State getState() const { return state_; }
    
private:
    State state_ = State::Uninitialized;
    int workCounter_ = 0;
};

// ==============================================================================
// Test Fixture for E2E Tests
// ==============================================================================

class SystemE2ETest : public ::testing::Test {
protected:
    void SetUp() override {
        fs_.clear();
    }
    
    void TearDown() override {
        if (app_.getState() == MockApplication::State::Running) {
            app_.shutdown();
        }
    }
    
    MockFileSystem fs_;
    MockConfigSystem config_;
    MockApplication app_;
};

// ==============================================================================
// E2E Test Cases
// ==============================================================================

TEST_F(SystemE2ETest, ApplicationLifecycle) {
    // Verify initial state
    EXPECT_EQ(app_.getState(), MockApplication::State::Uninitialized);
    
    // Initialize
    EXPECT_TRUE(app_.initialize(fs_, config_));
    EXPECT_EQ(app_.getState(), MockApplication::State::Running);
    
    // Run
    EXPECT_NO_THROW(app_.run());
    
    // Shutdown
    EXPECT_TRUE(app_.shutdown());
    EXPECT_EQ(app_.getState(), MockApplication::State::Stopped);
}

TEST_F(SystemE2ETest, DirectoriesCreatedOnInit) {
    app_.initialize(fs_, config_);
    
    EXPECT_TRUE(fs_.exists("data"));
    EXPECT_TRUE(fs_.exists("logs"));
    EXPECT_TRUE(fs_.exists("cache"));
}

TEST_F(SystemE2ETest, ConfigLoadedOnInit) {
    app_.initialize(fs_, config_);
    
    EXPECT_TRUE(config_.isLoaded());
}

TEST_F(SystemE2ETest, FileOperationsE2E) {
    // Setup
    app_.initialize(fs_, config_);
    
    // Create a file
    std::string testContent = "Hello, World!";
    EXPECT_TRUE(fs_.writeFile("data/test.txt", testContent));
    
    // Verify file exists
    EXPECT_TRUE(fs_.exists("data/test.txt"));
    
    // Read file back
    std::string readContent = fs_.readFile("data/test.txt");
    EXPECT_EQ(readContent, testContent);
}

TEST_F(SystemE2ETest, ConfigSystemE2E) {
    // Create config file
    fs_.writeFile("app.config", "database.host=localhost\ndatabase.port=5432");
    
    // Load config
    EXPECT_TRUE(config_.load(fs_, "app.config"));
    
    // Initialize app with config
    EXPECT_TRUE(app_.initialize(fs_, config_));
    
    // App should be running
    EXPECT_EQ(app_.getState(), MockApplication::State::Running);
}

TEST_F(SystemE2ETest, MultipleRunCycles) {
    app_.initialize(fs_, config_);
    
    // Run multiple cycles
    for (int i = 0; i < 5; ++i) {
        EXPECT_NO_THROW(app_.run());
        EXPECT_EQ(app_.getState(), MockApplication::State::Running);
    }
    
    app_.shutdown();
    EXPECT_EQ(app_.getState(), MockApplication::State::Stopped);
}

TEST_F(SystemE2ETest, CompleteWorkflow) {
    // Step 1: Setup filesystem
    fs_.createDirectory("project");
    fs_.writeFile("project/config.ini", "name=MyProject\nversion=2.0");
    
    // Step 2: Load configuration
    config_.load(fs_, "project/config.ini");
    ASSERT_TRUE(config_.isLoaded());
    
    // Step 3: Initialize application
    ASSERT_TRUE(app_.initialize(fs_, config_));
    ASSERT_EQ(app_.getState(), MockApplication::State::Running);
    
    // Step 4: Perform work
    fs_.writeFile("data/output.txt", "Processing complete");
    
    // Step 5: Verify output
    EXPECT_TRUE(fs_.exists("data/output.txt"));
    EXPECT_EQ(fs_.readFile("data/output.txt"), "Processing complete");
    
    // Step 6: Cleanup
    EXPECT_TRUE(app_.shutdown());
}

// ==============================================================================
// Note: main() is provided by gmock_main / gtest_main library
// ==============================================================================
