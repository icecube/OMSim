# Multi-threading mode
[TOC]

The OMSim-Framework allows for multi-threading. The number of threads can be controlled with the `--threads` argument (default 1). If you specify more threads than available, only the maximum available will be used.

## Introduction
Geant4 implements multi-threading using a master-worker model:

1. **Master Thread**: Responsible for initialization, geometry construction, and coordinating worker threads.
2. **Worker Threads**: Each simulates complete events independently.

Key points:
- Geometry and physics tables are shared read-only among threads.
- Each thread has its own instance of sensitive detectors, event and tracking managers.
- Random number generators are designed to produce independent streams for each thread.

## Thread Safety Guidelines

1. **Use Thread-Local Storage**: For data unique to each thread, use `G4ThreadLocal`.
2. **Protect Shared Resources**: Use mutex locks when accessing shared resources.
3. **Minimize Global Variables**: Prefer class members or local variables instead.
4. **Implement Thread-Safe Containers**: Ensure thread-safe access and modification of containers.

## Thread-Safe Global Instance Implementation


Both `OMSimHitManager` and `OMSimDecaysAnalysis` utilize a global instance pattern. This approach provides better control over the lifecycle of the instance and can prevent potential memory leaks when integrated into larger frameworks. The process works as follows:

1. Initialize the global instance explicitly at the start of the program. This is means, before the multi-threading starts. 
2. Access the instance through a global pointer.
3. Shut down and clean up the instance explicitly at the end of the program.

Implementation of the global instance pattern:

```cpp
// In the header file
class OMSimHitManager
{
public:
    static void init();
    static void shutdown();
    static OMSimHitManager& getInstance();
    // ... other methods ...

private:
    // ... other members ...
};

inline OMSimHitManager* g_hitManager = nullptr;

// In the source file
void OMSimHitManager::init()
{
    if (!g_hitManager) g_hitManager = new OMSimHitManager();
}

void OMSimHitManager::shutdown()
{
    delete g_hitManager;
    g_hitManager = nullptr;
}

OMSimHitManager& OMSimHitManager::getInstance()
{
    assert(g_hitManager);
    return *g_hitManager;
}
```

> **Note**: While this global instance implementation provides better control over the instance lifecycle, it requires explicit initialization and shutdown. Ensure these are called at appropriate times (single-thread) in your application (for example in main before/after run).

### Example: OMSimHitManager

The `OMSimHitManager` class demonstrates several thread-safety techniques for saving data:

```cpp
class OMSimHitManager
{
public:
    void appendHitInfo(/* parameters */);
    void mergeThreadData();
    // ... other methods ...

private:
    static G4Mutex m_mutex;  // Mutex for thread synchronization

    struct ThreadLocalData {
        std::map<G4int, HitStats> moduleHits;
    };
    // Thread-local storage for hit data
    G4ThreadLocal static ThreadLocalData* m_threadData;

    // ... 
};
```

Key features:
- Thread-local storage for hit data (`m_threadData`), each thread will start one
- Mutex (`m_mutex`) for thread synchronization.


The `appendHitInfo` method is used by all threads and uses to the thread-local `m_threadData`:

```cpp
void OMSimHitManager::appendHitInfo(/* parameters */)
{
    if (!m_threadData)
    {
        // Initialize thread-local data on first use
        m_threadData = new ThreadLocalData();
    }

	auto &moduleHits = m_threadData->moduleHits[p_moduleNumber];
	G4int eventID = G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID();
	moduleHits.eventId.push_back(eventID);
    //...
}
```

The `mergeThreadData` method combines data from all threads into a single vector:

```cpp
void OMSimHitManager::mergeThreadData()
{
    G4AutoLock lock(&m_mutex);  // Ensure thread-safe access to shared data
    if (m_threadData)
    {
        // Merge thread-local data into a single container
        // This is where data from all threads is combined
        // ...

        // Clean up thread-local data after merging
        delete m_threadData;
        m_threadData = nullptr;
    }
}
```

> **Important**: Call `mergeThreadData` after all threads have finished simulating (after a run has completed).

### Example: Saving Data Per Thread

In scenarios where merging data is unnecessary, or the amount of data is too large to wait until end of run, each thread can save its data in separate files. This is demonstrated in the `OMSimDecaysAnalysis` class.


1. **appendDecay**: Collects decay data for each thread.

```cpp
//from radioactive_decays/src/OMSimDecaysAnalysis.cc
void OMSimDecaysAnalysis::appendDecay(G4String pParticleName, G4double pDecayTime, G4ThreeVector pDecayPosition)
{
    if (!m_threadDecayStats)
    {
        m_threadDecayStats = new DecayStats();
    }
    G4int lEventID = G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID();
    m_threadDecayStats->eventId.push_back(lEventID);
    (...)
}
```

2. **writeThreadDecayInformation**: Writes decay data to a file specific to each thread.

```cpp
//from radioactive_decays/src/OMSimDecaysAnalysis.cc
void OMSimDecaysAnalysis::writeThreadDecayInformation()
{
    G4String outputSuffix = OMSimCommandArgsTable::getInstance().get<std::string>("output_file");
    G4String decaysFileName = outputSuffix + "_" + Tools::getThreadIDStr() + "_decays.dat"; // one file per thread, appending thread id to file name

    std::fstream dataFile;
    dataFile.open(decaysFileName.c_str(), std::ios::out | std::ios::app);
    if (m_threadDecayStats->eventId.size() > 0)
    {
        (...)
    }
    dataFile.close();
}
```

Data is saved after each event in the `EndOfEventAction` method to handle large volumes of data:

```cpp 
//from radioactive_decays/src/OMSimEventAction.cc
void OMSimEventAction::EndOfEventAction(const G4Event *p_evt)
{
    if (!OMSimCommandArgsTable::getInstance().get<bool>("multiplicity_study"))
    {
        OMSimDecaysAnalysis &analysisManager = OMSimDecaysAnalysis::getInstance();
        analysisManager.writeThreadHitInformation();
        analysisManager.writeThreadDecayInformation();
        analysisManager.reset();
    }
}
```

As you can see, in case of multiplicity study, we need to merge the data, as we are looking for coincidences. In that case we have to merge:
```cpp
//from radioactive_decays/OMSim_radioactive_decays.cc
		if (lArgs.get<bool>("multiplicity_study"))
		{
			G4double coincidenceTimeWindow = lArgs.get<double>("multiplicity_time_window")*ns;
			analysisManager.writeMultiplicity(coincidenceTimeWindow);
			analysisManager.reset();
		}
```        


## Best Practices for Creating New Thread-Safe Containers

When implementing new thread-safe containers in Geant4:

1. **Use Thread-Local Storage**: 
   ```cpp
   // Declare thread-local storage
   G4ThreadLocal static YourDataType* threadLocalData = nullptr;
   ```

2. **Initialize on First Use**:
   ```cpp
   if (!threadLocalData) {
       // Initialize thread-local data only when first accessed
       threadLocalData = new YourDataType();
   }
   ```

3. **Implement Data Merging (if necessary)**: 
   ```cpp
   void mergeThreadData() {
       G4AutoLock lock(&m_mutex);  // Ensure thread-safe access
       // Merge threadLocalData into a global container
       // This is where you combine data from all threads
   }
   ```

4. **Ensure Proper Cleanup**:
   ```cpp
   void reset() {
       // Clean up thread-local data
       delete threadLocalData;
       threadLocalData = nullptr;
   }
   ```

5. **Implement Thread-Safe Access**:
   Use mutex locks for shared resource access:
   ```cpp
   G4AutoLock lock(&m_mutex);  // Protect access to shared resources
   // Access or modify shared resources
   ```

By following these guidelines and studying the provided examples, you can create thread-safe containers and classes for your Geant4 simulations, ensuring proper behavior in multi-threaded environments.


## Troubleshooting Multi-threading Issues

When developing new code with multi-threaded simulations in Geant4, you may encounter race conditions or other thread-related issues. Here's a general approach to diagnose and resolve these problems:

### 1. Use Valgrind Tools
Valgrind provides powerful tools for detecting thread-related issues:

a) Helgrind:
   ```
   valgrind --log-file="output_helgrind.txt" --tool=helgrind ./OMSim_* [arguments]
   ```

b) DRD (Data Race Detector):
   ```
   valgrind --log-file="output_helgrind.txt" --tool=drd ./OMSim_* [arguments]
   ```

These tools can identify potential race conditions and other thread-related issues.
### 2. Analyse the Output
- Review the Valgrind output carefully. Look for:
- Data race warnings
- Mutex-related issues
- Potential deadlocks
- Tip: Use an LLM (like ChatGPT) to help interpret complex error messages and suggest potential solutions.

### 4. Modify and repeat
- Once you identify the object/method causing the error, check if it's obviously not thread-safe and being shared during simulation.
- For a deeper understanding, provide the complete class (header + source) to the LLM for more detailed guidance.
- Make changes to address the identified issues.
- Use Valgrind tools again to verify if the issues have been resolved.



