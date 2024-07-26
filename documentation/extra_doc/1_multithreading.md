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

## Thread-Safe Singleton Implementation

Both `OMSimHitManager` and `OMSimDecaysAnalysis` utilize the singleton pattern with a thread-safe `getInstance` method. This method employs a double-checked locking pattern to ensure thread safety while minimizing synchronization overhead. The process works as follows:

1. Check if the instance exists without locking.
2. If the instance doesn't exist, acquire a lock.
3. After acquiring the lock, check again if the instance exists (to handle potential creation by another thread during the lock wait). Without this check, multiple threads could pass the first check and reach the lock. Once inside the locked section, each thread would create a new instance if the second check wasn't there, potentially leading to multiple instances.
4. If the instance still doesn't exist, create it.

Implementation of the `getInstance` method:

```cpp
{
    if (!mInstance)  // First check without locking
    {
        G4AutoLock lock(&mMutex);  // Lock if instance doesn't exist
        if (!mInstance)  // Double-check after locking
        {
            mInstance = new OMSimHitManager();  // Create instance if it still doesn't exist
        }
    }
    return *mInstance;
}
```

This approach guarantees the creation of only one instance, even in a multi-threaded environment, while ensuring thread-safe initialization. The use of `G4AutoLock` and a static mutex provides proper synchronization across threads.

> **Note**: While this singleton implementation is thread-safe, consider whether a singleton is the best design choice for your specific use case. Singletons can sometimes lead to issues with testability and create hidden dependencies.


### Example: OMSimHitManager

The `OMSimHitManager` class demonstrates several thread-safety techniques:

```cpp
class OMSimHitManager
{
public:
    static OMSimHitManager& getInstance();
    void appendHitInfo(/* parameters */);
    void mergeThreadData();
    // ... other methods ...

private:
    static G4Mutex mMutex;  // Mutex for thread synchronization
    static OMSimHitManager* mInstance;  // Singleton instance

    struct ThreadLocalData {
        std::map<G4int, HitStats> moduleHits;
    };
    // Thread-local storage for hit data
    G4ThreadLocal static ThreadLocalData* mThreadData;

    // ... constructor and destructor ...
};
```

Key features:
- Singleton pattern for global access (consider if necessary for your use case).
- Thread-local storage for hit data (`mThreadData`).
- Mutex for thread synchronization.


The `appendHitInfo` method:

```cpp
void OMSimHitManager::appendHitInfo(/* parameters */)
{
    if (!mThreadData)
    {
        // Initialize thread-local data on first use
        mThreadData = new ThreadLocalData();
    }

    // Append hit information to thread-local container
    // This is thread-safe as each thread has its own mThreadData
    // ... append hit information to mThreadData->moduleHits ...
}
```

The `mergeThreadData` method combines data from all threads:

```cpp
void OMSimHitManager::mergeThreadData()
{
    G4AutoLock lock(&mMutex);  // Ensure thread-safe access to shared data
    if (mThreadData)
    {
        // Merge thread-local data into a single container
        // This is where data from all threads is combined
        // ...

        // Clean up thread-local data after merging
        delete mThreadData;
        mThreadData = nullptr;
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
    if (!mThreadDecayStats)
    {
        mThreadDecayStats = new DecayStats();
    }
    G4int lEventID = G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID();
    mThreadDecayStats->eventId.push_back(lEventID);
    (...)
}
```

2. **writeThreadDecayInformation**: Writes decay data to a file specific to each thread.

```cpp
//from radioactive_decays/src/OMSimDecaysAnalysis.cc
void OMSimDecaysAnalysis::writeThreadDecayInformation()
{
    G4String lOutputSuffix = OMSimCommandArgsTable::getInstance().get<std::string>("output_file");
    G4String lDecaysFileName = lOutputSuffix + "_" + getThreadIDStr() + "_decays.dat"; // one file per thread, appending thread id to file name

    std::fstream lDatafile;
    lDatafile.open(lDecaysFileName.c_str(), std::ios::out | std::ios::app);
    if (mThreadDecayStats->eventId.size() > 0)
    {
        (...)
    }
    lDatafile.close();
}
```

Data is saved after each event in the `EndOfEventAction` method to handle large volumes of data:

```cpp 
//from radioactive_decays/src/OMSimEventAction.cc
void OMSimEventAction::EndOfEventAction(const G4Event *evt)
{
    if (!OMSimCommandArgsTable::getInstance().get<bool>("multiplicity_study"))
    {
        OMSimDecaysAnalysis &lAnalysisManager = OMSimDecaysAnalysis::getInstance();
        lAnalysisManager.writeThreadHitInformation();
        lAnalysisManager.writeThreadDecayInformation();
        lAnalysisManager.reset();
    }
}
```

As you can see, in case of multiplicity study, we need to merge the data, as we are looking for coincidences. In that case we have to merge:
```cpp
//from radioactive_decays/OMSim_radioactive_decays.cc
		if (lArgs.get<bool>("multiplicity_study"))
		{
			G4double lCoincidenceTimeWindow = lArgs.get<double>("multiplicity_time_window")*ns;
			lAnalysisManager.writeMultiplicity(lCoincidenceTimeWindow);
			lAnalysisManager.reset();
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
       G4AutoLock lock(&mMutex);  // Ensure thread-safe access
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
   G4AutoLock lock(&mMutex);  // Protect access to shared resources
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





































