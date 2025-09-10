#  Course Registration Portal 

## Overview

The **Course Registration Portal (Academia)** is a **client–server based system** developed in C.
It simulates a university registration portal where **Admins**, **Faculty**, and **Students** can perform their respective operations.

The project uses **socket programming** for communication between a **server** (`server.c`) and multiple **clients** (`client.c`).
All data is persisted in `.dat` files (`students.dat`, `faculty.dat`, `courses.dat`).




## Project Structure

```
├── server.c        # Main server application handling logic & file operations
├── client.c        # Client interface (Admin, Faculty, Student)
├── students.dat    # Student database
├── faculty.dat     # Faculty database
├── courses.dat     # Course database
```



##  Roles & Functionalities

###  Admin

* Add/ Update/ Activate/ Block students
* Add/ Update faculty details
* View student and faculty records

### Faculty

*  Add/ Remove courses
*  Update course details (name, seat limit)
*  View enrollments for offered courses
*  Change password

###  Student

*  Enroll in courses
*  Drop courses
*  View all available courses
*  View enrolled courses
*  Change password



##  Server Functionalities

Key functions implemented in `server.c`:

* **Admin Ops** : `add_student()`, `add_faculty()`, `view_student_details()`, `block_student()`, `activate_student()`, `modify_student_details()`, `modify_faculty_details()`
* **Faculty Ops** : `add_course()`, `remove_course()`, `update_course_details()`, `view_offering_courses()`
* **Student Ops** : `view_all_courses()`, `enroll_course()`, `drop_course()`, `view_enrolled_courses()`
* **General Ops** : `change_student_password()`, `change_faculty_password()`
* **File Ops** : `load_data()`, `save_data()`
* **Networking** : `handle_client()`

##  How to Run

1. **Compile the server and client programs**

   ```bash
   gcc server.c -o server
   gcc client.c -o client
   ```

2. **Run the server** (default port: `8080`)

   ```bash
   ./server
   ```

3. **Run the client** (in another terminal)

   ```bash
   ./client
   ```

4. **Login** with your role credentials:

   *  Admin : manage students & faculty
   *  Faculty : manage courses
   *  Student : enroll/drop courses



##  Security

* Password-protected login system
* Invalid credentials leads to connection being terminated
* Students cannot exceed course seat limits
* Faculty/ Admin operations require authentication



## Author

**Ankith Kini**

