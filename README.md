# Time domain audio processing
This library consists of algorithms and tools to do audio processing in the time domain. This in contrast to processing in the *frequency domain*. Even though this is a digital signal processing (DSP) library, the approach is &ldquo;analogue&rdquo;.

## Declaration and implementation
The majority of the library is constructed using templates. To facilitate tweaking, declaration and implementation of many non-trivial cases reside in different files. By default, the declaration header includes the implementation header, that is named `declaration-header-implementation.hpp`. This behaviour can be disabled by defining `TDAP_INCLUDE_NO_IMPLEMENTATION`. This way, it is possible to include the interface headers in one part of a project and do template instantiation in another part.   

## History
A lot of the algorithms and tools are several years old and have been used in different form in other projects. These other projects often reinvented many wheels that are now a standard part of the C++ specification (2017) or are readily available in libraries like Boost. This project aims to stop reinventing wheels and focus on the actual processing algorithms. 

Where this library borrows from other sources, it will try to honour these and mention the involved licenses. This happens even if the code was completely refactored but the essence of the algorithm remains unchanged.