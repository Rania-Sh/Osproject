# Osproject
# Graph Movement Simulation Project 🚀

פרויקט זה מציג סימולציה ויזואלית של תנועה בגרף ומציאת המסלול הקצר ביותר. המערכת כוללת ממשק משתמש אינטראקטיבי המאפשר שליטה באנימציה ומעקב אחר התקדמות הסוכן בין הצמתים.

## 👥 חברי הצוות
* **Hadeel Shehadeh** - חברת צוות 1.
* **Rania Shqerat** - חברת צוות 2.
* **Mohanad Masri** - חבר צוות 3.

## 🛠 פרטי המימוש (Implementation Details)

### אבני דרך 1 ו-2 (Milestones 1 & 2)
* **ייצוג הגרף**: שימוש במטריצת סמיכויות (Adjacency Matrix).
* **אלגוריתמיקה**: מימוש אלגוריתם דייקסטרה (Dijkstra) למציאת המסלול הקצר ביותר.
* **ויזואליזציה**: שימוש בספריית **Raylib** להצגת הגרף על המסך.

### אבן דרך 3 (Milestone 3 - הנוכחית)
* **לוגיקת תנועה**: חלוקת כל קשת ל-W "קפיצות" בהתאם למשקל הקשת.
* **תזמון (Timing)**: כל קפיצה אורכת 300ms.
* **השהיה**: עצירה של שנייה אחת (1-second delay) בכל צומת ביניים במסלול.
* **ניהול אנימציה**: שילוב מערכת ניהול מצבים (State Management) לטיפול ב-Play/Stop, השהיה ותצוגת סיום.
#3##--- 
## לוגיקת אנימציה ותזמון — הדיל שחאדה
הייתי אחראית על פיתוח מנוע האנימציה ולוגיקת בניית המסלול של הסימולציה. העבודה שלי מבטיחה שתנועת הישות עוקבת במדויק אחר דרישות התזמון והקפיצות שהוגדרו במפרט הפרויקט.
## תרומות מרכזיות:
* בניית מסלול דינמי: 
פיתחתי את הפונקציה buildAnimationPath הממירה את רשימת הצמתים (Nodes) שהתקבלה מאלגוריתם דייקסטרה לסדרה של צעדי אנימציה בדידים.
* לוגיקת קפיצות מבוססת משקל:
מימשתי מערכת "קפיצות" שבה מספר הצעדים בין שני צמתים נקבע לפי משקל הקשת ($W$). כל קפיצה מתוזמנת להימשך בדיוק 300 מילי-שניות.
* מנגנון השהיה בצמתים:
הטמעתי לוגיקת עצירה של שנייה אחת בכל פעם שהישות מגיעה לצומת ביניים (צומת שאינו נקודת ההתחלה או הסיום), כפי שנדרש במפרט אבני הדרך.
* אינטרפולציה ליניארית (Lerp): 
השתמשתי בחישובים מתמטיים כדי לחשב מעברי קואורדינטות $(x, y)$ חלקים בין הצמתים, כך שהישות נעה בצורה זורמת על המסך.
* ניהול זיכרון:
יצרתי מבני נתונים יציבים לאחסון רצף האנימציה ומימשתי את הפונקציה freeAnimationPath כדי להבטיח שאין דליפות זיכרון במהלך הסימולציה.

## אלגוריתם דייקסטרה (Dijkstra's Algorithm)
בנוסף ללוגיקת האנימציה, מימשתי את התשתית האלגוריתמית למציאת המסלול הקצר ביותר.
## הפעולות שביצעתי:
* מימוש אלגוריתם דייקסטרה:
כתבתי את הקוד ב-dijkstra.c שמוצא את המסלול האופטימלי בין שני צמתים בגרף ממושקל.
* ניהול קלטים:
המערכת שפיתחתי קוראת קבצי טקסט במבנה של גרף (צמתים, קשתות ומשקלים) ומפענחת אותם למטריצת שכנויות.
* שחזור מסלול:
השתמשתי במערך parent כדי לשחזר ולהדפיס את המסלול המדויק בפורמט הנדרש (למשל: 0->2->5).
* טיפול בשגיאות:
הוספתי בדיקות למקרים של משקלים שליליים או חוסר במסלול בין צמתים כדי למנוע קריסות של התוכנית.
* אוטומציה ב-Makefile:
עדכנתי את קובץ ה-Makefile כך שיכלול את המטרה make milestone1, המאפשרת קימפול מהיר ובדיקה של האלגוריתם.
## איך לקמפל בטרמינל
```bash
make milestone1
```
הפקודה הזו תיצור קובץ הרצה בשם dijkstra.
## איך להריץ את הקוד
כדי להריץ את התוכנית על קובץ גרף (למשל test_graph.txt), נשתמשי בפקודה הבאה:
```bash
./dijkstra test_graph.txt
```

## 👩‍💻 My Contribution (Rania Shqerat)

במסגרת הפרויקט, הייתי אחראית על פיתוח צד הלקוח (Frontend), ממשק המשתמש ולוגיקת האנימציה המתקדמת:

### 🎨 UI/UX & Graphics
*   **ממשק משתמש (User Interface)**: תכננתי ומימשתי ממשק גרפי מלא ב-Dark Mode תוך שימוש בספריית **Raylib**.
*   **מערכת כפתורים אינטראקטיבית**: פיתחתי כפתורי שליטה (Start, Pause, Resume, Reset) המאפשרים אינטראקציה מלאה עם הסימולציה.
*   **שכבת תצוגה (Status Overlay)**: הוספתי רכיבי טקסט דינמיים המציגים את מצב המערכת הנוכחי בזמן אמת.

### ⚙️ Animation Logic (Milestone 3)
*   **ניהול מצבי אנימציה (State Management)**: מימשתי את לוגיקת המעברים בין מצבי התנועה, העצירה והסיום.
*   **מנגנון תנועה (Movement Logic)**: פיתחתי את המנגנון המחלק כל קשת ל-$W$ קפיצות בהתאם למשקל הקשת, עם תזמון של 300ms לכל קפיצה.
*   **בקרת השהיה**: הטמעתי השהיה של שנייה אחת בכל צומת מעבר כדי לדמות תנועה ריאליסטית בגרף
   ## Milestone 4 - Multi Traveler

### קימפול:
make milestone4

### הרצה:
./sim test_graph.txt

### תיאור:
מספר נוסעים נעים בו-זמנית על הגרף.
תהליך האב מחשב מסלולי דייקסטרה ומציג את כל הנוסעים בצבעים שונים.
כל נוסע הוא תהליך בן שנוצר עם fork().
האב שולח SIGTERM לכל בן בסיום מסלולו..

### 🛠️ Software Engineering & Infrastructure
*   **ניהול גרסאות (GitHub)**: ניהלתי את מאגר הקוד, הגדרתי את מבנה הפרויקט וסידרתי את קבצי המקור וה-Build.
*   **סביבת עבודה (Build System)**: הגדרתי את קבצי ה-**CMake** וה-**Makefile** כדי לאפשר קומפילציה חלקה בסביבת לינוקס.
*   **בדיקות ודיבאגינג**: ביצעתי בדיקות זיכרון עם **Valgrind** כדי לוודא עבודה נקייה ללא דליפות זיכרון בשרת ובאנימציה.

---

## 💻 הוראות בנייה והרצה (Compile and Run)
כדי לעבוד עם אבן הדרך השלישית, הריצי את הפקודות הבאות בטרמינל:
```bash
# קימפול אבן הדרך השלישית
make milestone3

# הרצת הסימולציה (יש לספק קובץ גרף)
./sim test_graph.txt

# ניקוי קבצי הבנייה
make clean
```

## Milestone 5 – IPC (Pipes)

### Compile:
make milestone5

### Run:
./sim test_graph.txt

### Description:
Each child process independently reads the graph file and computes its own
Dijkstra path. It then travels node-by-node, sending a small struct message
to the parent over a dedicated pipe after each arrival.
The parent reads these messages non-blockingly inside the raylib GUI loop,
prints the log, and updates each traveler's dot on screen.




# Milestone 6 – My Work Explanation

## Overview

In this milestone, I worked on adding synchronization between travelers in the graph simulation.

The project was already working up to Milestone 5, where each traveler runs as a separate child process, calculates its own shortest path using Dijkstra, and communicates with the parent process using pipes.

My work in Milestone 6 was to extend that behavior so that travelers cannot enter the same graph node at the same time.

---

## Files I Worked On

### `main6.c`

I created and worked on `main6.c` as the implementation file for Milestone 6.

This file is based on the working Milestone 5 logic, but I added synchronization logic using POSIX named semaphores.

### `Makefile`

I added a new build target:

```makefile
milestone6:
	$(CC) $(CFLAGS) main6.c animation.c animationui.c dijkstra.c -o sim $(LIBS)
```

This allows Milestone 6 to be built using:

```bash
make milestone6
```

### `test_graph_milestone6.txt`

I added a special test graph file for Milestone 6.

This file creates a situation where three travelers move toward the same shared node. This makes the synchronization behavior visible in the GUI, because some travelers must wait outside the occupied node.

---

## IPC Used

The project continues to use pipes as the IPC mechanism.

Each child process sends messages to the parent process through a pipe. The parent receives these messages and updates the GUI.

The messages include:

* `MSG_ENTERED`: the traveler entered a node.
* `MSG_WAITING`: the traveler is waiting outside a busy node.
* `MSG_FINISHED`: the traveler finished its path.

The parent process is responsible for printing the logs and updating the visual state of each traveler.

---

## Synchronization Method

For synchronization, I used POSIX named semaphores.

Each graph node has one semaphore. The semaphore is initialized with value `1`, which means only one traveler can enter that node at a time.

Before a traveler enters a node, it tries to acquire the semaphore for that node.

If the node is free, the traveler enters it.

If the node is already occupied, the traveler sends a `MSG_WAITING` message to the parent and waits until the semaphore becomes available.

After entering the node, the traveler stays inside it for one second using:

```c
sleep(1);
```

Then the traveler releases the semaphore so another waiting traveler can enter.

---

## Waiting State in the GUI

I added a visible waiting state in the GUI.

When a traveler is waiting outside an occupied node:

* The traveler is shown in orange.
* A `WAIT` label is displayed.
* The traveler is drawn near the node it is waiting for.

This makes the synchronization behavior clear during the simulation.

---

## How to Build and Run

To build Milestone 6:

```bash
make clean
make milestone6
```

To run the Milestone 6 demonstration file:

```bash
./sim test_graph_milestone6.txt
```

This test file shows three travelers reaching the same shared node, where only one traveler enters at a time and the others wait.

---

## What This Demonstrates

This milestone demonstrates:

* Multi-process execution using `fork()`.
* IPC using pipes.
* Node-level synchronization using POSIX named semaphores.
* Preventing more than one traveler from being inside the same node at the same time.
* Displaying waiting travelers visually in the GUI.
* Preserving the parent process as the controller of the GUI and logs.

---

## Notes

The original Milestone 5 behavior is preserved in `main5.c`.

Milestone 6 is implemented separately in `main6.c` so that the previous milestone remains available and unchanged.

بpd
### IPC choice: pipes
Pipes were chosen for their simplicity and zero-setup overhead.
One pipe per child (child writes, parent reads). The read end is set to
O_NONBLOCK so the GUI loop never blocks waiting for a child message.
