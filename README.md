# Smart-Car-Parking-System
Project Overview
This project is a smart car parking management system for a lot with 50 parking spaces, implemented in C using advanced data structures like AVL Tree / B Tree / B+ Tree and file handling instead of arrays.

The system automates:
Parking space allocation based on membership
Vehicle registration and tracking
Billing and membership upgrades
Revenue and usage analytics using tree-based structures

🛠️ Tools & Technologies
C Language
File Handling for persistent storage (no arrays used)
B+ Tree for efficient data management

🎯 Features
✅ Entry Operations
Register new vehicle with:
Vehicle number (unique), owner name
Arrival time (auto-recorded)
Initial membership and parking hours = 0
Allocate parking space based on membership tier:
Golden: Space 1–10 (nearest)
Premium: Space 11–20
Regular: First available from space 21–50
For existing vehicles, update arrival time and space status

✅ Exit Operations
Auto-calculate parking hours
Add to total parking hours

Update membership:
100+ hrs → Premium
200+ hrs → Golden

Compute billing:
₹100 for first 3 hours, ₹50/hr after
10% discount for members
Free up allocated parking space

✅ Analytics & Reports
Sort and display vehicles based on:
Total parkings
Total amount paid
Sort and display parking spaces based on:
Occupancy frequency
Revenue generated
Query vehicles by amount paid within a given range

📁 Data Storage:
Initial data for 20 parking spaces (with mixed membership levels) is generated using file handling

All updates (vehicle records, parking space status, membership, billing) persist to file
📌 Instructions to Run
1.Compile the code using a C compiler (e.g., gcc):
gcc parking_system.c -o parking_system
2.Run the program:
./parking_system
3.Follow on-screen menu to simulate vehicle entry, exit, and reporting.

Developer Contribution :
Designed and implemented parking space management logic
Developed AVL/B/B+ Tree structures from scratch
Integrated membership and billing policies
Implemented file-based persistent storage

✅ Advantages
Fully dynamic structure using B/B+ Tree or AVL Tree
No arrays used — ensures flexible, scalable data handling
Simulates real-world database behavior using file-based persistence
Membership and billing handled automatically with customizable policies
