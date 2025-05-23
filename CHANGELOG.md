# Implemented functionality
- Command line parameter parser
  - Base parameters
  - Help parameters
  - Invalid input handling
  - Fill configuration structure
- Protocol factory static method and setup
  - Instantiate transport protocol
  - DNS query to get the host server IPv4 address
  - Method to create socket coressponding to chosen transport protocol
- Client
  - Client I/O handler
    - Command factory
    - Concrete Command execution
  - Client network message processing
  - Client network message queue processing
  - Client error handling
- Protocol
  - Await certain messages
  - Implement message queue
  - Ignore duplicate or incomplete messages
- TCP
  - Implement server connection method 
  - Implement basic communication methods
  - Segmentation/fragmentation protection
  - Preprocess messages from buffer
  - Shutdown socket connection properly
  - Get message content
  - Check for CRLF
- UDP
  - Implement basic communication methods
  - Confirm messages
  - Preprocess messages from buffer
  - Bind message IDs
  - Extract message IDs
  - Get message content
 
# Possible Issues
- Client state machine may be innacurate in certain situations.
- 
# Issues
- UDP messages order may be random and protocol message queue and await method does not take it into account.
