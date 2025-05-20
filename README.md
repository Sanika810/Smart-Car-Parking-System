# Smart Car Parking Lot System

Welcome to the **Smart Car Parking Lot System**, a C-based application designed to manage a car parking lot using efficient data structures and logical flow. This system utilizes **B+ Trees** for storing and managing both vehicles and parking spaces, enabling quick insertion, deletion, and range search operations.

## Features

*  **Vehicle Entry and Exit**
*  **Sorting and Analysis** based on:

  * Number of parkings done
  * Amount paid by the vehicle (range search)
  * Occupancy frequency of parking spaces
  * Revenue generated per space
*  **B+ Tree** based dynamic insertion and management
* File persistence with:

  * `vehicle_data.txt`
  * `parking_data.txt`

## Data Structures

* **vehicleNode** and **spaceNode** represent the B+ Tree nodes for vehicles and parking spaces, respectively.
* These trees are dynamically loaded from files at the start and updated back on changes.

## Files Used

* `vehicle_data.txt` – Stores vehicle information
* `parking_data.txt` – Stores parking space data

## How to Use

1. Compile the program using a C compiler like GCC:

   ```sh
   gcc -o parking_system main.c
   ```
2. Run the executable:

   ```sh
   ./parking_system
   ```
3. Interact with the system using the menu:

   * Enter vehicle details for parking
   * Exit a vehicle and compute payment
   * Sort data or perform analysis
   * Exit and save all data

## Highlights

* **Automatic timestamping** using `time(NULL)` on entry and exit.
* **File Persistence**: Automatically saves all changes back to data files after every update.
* **Range Search**: Efficient search of vehicles based on payment amount range.

## Memory Management

* All dynamically allocated memory (for B+ Trees) is freed before exit.

## Requirements

* C compiler (e.g., GCC)
* Compatible with Windows/Linux

## Status Codes

* All major operations return a `statusCode` that indicates success or failure.

## Example Menu Output

```
1. Vehicle Entry
2. Vehicle Exit
3. Sorting and analysis
4. Exit system
```

## Example Input

```
Enter your vehicle details
Enter the vehicle number:
1001
Enter the name of owner:
John Doe

Enter your vehicle number:
1001

Enter the amount range
minimum amount: 50
maximum amount: 500
```

## Author

Developed as part of a smart parking lot simulation using fundamental data structures and logical modeling.

---

Feel free to extend or modify this system to add more features such as reservation, graphical interface, or remote database support!
