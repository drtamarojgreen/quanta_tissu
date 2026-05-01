PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE Article (
 id integer PRIMARY KEY,
 author text NOT NULL,
 authorDate text NOT NULL,
 title text NOT NULL,
 description text NOT NULL,
 content text NOT NULL);
INSERT INTO Article VALUES(1,'Tamaro Green','January 31, 2021','African American History Month','Resources for African American History Month',replace(replace('February is African-American History Month and to honour the people of African descent who contributed to America, TJG Web Services is spending the first week of February to provide resources for the history of African-Americans.  There is no login necessary for this cms platform so please do not post copyrighted material and source references if necessary.  This is a platform also for which the rules of netiquette should apply.  The articles posted here will be stored for future sessions.  \r\n','\r',char(13)),'\n',char(10)));
CREATE TABLE Review (
 id integer PRIMARY KEY,
 author text NOT NULL,
 authorDate text NOT NULL,
 title text NOT NULL,
 description text NOT NULL,
 content text NOT NULL);
CREATE TABLE AdministratorGroup (
id integer PRIMARY KEY,
groupName TEXT NOT NULL
);
CREATE TABLE Administrator (
id integer PRIMARY KEY,
administratorName text NOT NULL,
administratorGroupId integer NOT NULL,
FOREIGN KEY (administratorGroupId)
REFERENCES AdministratorGroup (id)
ON UPDATE CASCADE
ON DELETE CASCADE
);
CREATE TABLE Artist(
id INTEGER PRIMARY KEY,
artistName TEXT
);
CREATE TABLE Chat (
id INTEGER PRIMARY KEY,
userIdFrom INTEGER NOT NULL,
userIdTo INTEGER NOT NULL,
dateTime TEXT NOT NULL,
priority INTEGER,
subject TEXT,
message TEXT
);
INSERT INTO Chat VALUES(1,644,0,'0','Customer Service Request','Customer Service Request',NULL);
INSERT INTO Chat VALUES(2,1447,0,'19/01/2021 17:04:53',0,'Customer Service Request','Customer Service Request');
CREATE TABLE Lecture(
instructor TEXT,
lectureName TEXT,
lecturePoster BINARY,
PRIMARY KEY(instructor,lectureName)
);
CREATE TABLE LectureNote(
id INTEGER,
noteInstructor TEXT,
noteLecture TEXT,
noteText TEXT,
FOREIGN KEY(noteInstructor, noteLecture) 
REFERENCES Lecture(instructor, lectureName)
);
CREATE TABLE Lecturer(
id INTEGER PRIMARY KEY,
lecturerName TEXT
);
CREATE TABLE School(
id INTEGER PRIMARY KEY,
schoolName TEXT,
schoolLecturer INTEGER 
REFERENCES Lecturer(id) 
ON UPDATE CASCADE
);
CREATE TABLE Course
( id INTEGER PRIMARY KEY,
courseName text NOT NULL);
CREATE TABLE Journal (
id integer PRIMARY KEY,
journal text NOT NULL);
CREATE TABLE Subscriber (
id integer PRIMARY KEY,
subscriber text NOT NULL);
CREATE TABLE Subscription (
id integer PRIMARY KEY,
subscriptionPlan text NOT NULL,
publisher text NOT NULL,
topic text NOT NULL);
CREATE TABLE Student
( id INTEGER PRIMARY KEY,
lastName VARCHAR NOT NULL,
firstName VARCHAR,
courseId INTEGER,
CONSTRAINT fk_Course
FOREIGN KEY (courseId)
REFERENCES Course(id)
ON DELETE CASCADE
);
CREATE TABLE HrGroup (
id integer PRIMARY KEY,
groupName TEXT NOT NULL
);
CREATE TABLE HrClient (
id integer PRIMARY KEY,
clientFirstName text NOT NULL,
clientLastName text NOT NULL,
clientSpecialty text NOT NULL,
clientContact text NOT NULL,
hrGroupId integer NOT NULL,
FOREIGN KEY (hrGroupId)
REFERENCES HrGroup (id)
ON UPDATE CASCADE
ON DELETE CASCADE);
CREATE TABLE HrEmployer (
id integer PRIMARY KEY,
employerName text NOT NULL,
employerContact text NOT NULL,
employerContactType text NOT NULL,
employerContactInfo text NOT NULL,
hrGroupId integer NOT NULL,
FOREIGN KEY (hrGroupId)
REFERENCES HrGroup (id)
ON UPDATE CASCADE
ON DELETE CASCADE
);
CREATE TABLE Researcher (
id integer PRIMARY KEY,
researcherFirstName TEXT NOT NULL,
researcherLastName TEXT NOT NULL,
researcherDegree TEXT NOT NULL,
researcherMajor TEXT NOT NULL,
researcherInstitution TEXT NOT NULL,
researcherSpecialty TEXT NOT NULL
);
INSERT INTO Researcher VALUES(1,'John','Doe','Bachelor of Science','Investigative Journalism','II Data School','Investigative Journalism');
CREATE TABLE Topic (
id integer PRIMARY KEY,
topicName text NOT NULL,
topicSubject text NOT NULL,
topicDescription text NOT NULL,
researcherId integer NOT NULL,
FOREIGN KEY (researcherId)
REFERENCES Researcher (id)
ON UPDATE CASCADE
ON DELETE CASCADE
);
CREATE TABLE Project (
id integer PRIMARY KEY,
projectName text NOT NULL,
projectSubject text NOT NULL,
projectDescription text NOT NULL,
researcherId integer NOT NULL,
FOREIGN KEY (researcherId)
REFERENCES Researcher (id)
ON UPDATE CASCADE
ON DELETE CASCADE
);
CREATE TABLE Video(
id integer PRIMARY KEY,
artist TEXT,
videoName TEXT,
videoPath TEXT);
CREATE TABLE Event (
id integer PRIMARY KEY,
title text NOT NULL,
startDate text NOT NULL,
endDate text NOT NULL,
location text NOT NULL,
description text NOT NULL);
INSERT INTO Event VALUES(1,'Data Journalism Management Conference','November 30, 2020','December 4, 2020','Conference','Data Journalism Management Conference');
INSERT INTO Event VALUES(2,'Management Artificial Intelligence Conference','January 4, 2021','January 8, 2021','TJG Web Conferences','MAIC 2021 - Management Artificial Intelligence Conference');
INSERT INTO Event VALUES(3,'Humanity and Technology Conferences','January 11, 2021','January 15, 2021','TJG Web Conferences','HATC 2021 - Humanity and Technology Conference');
CREATE TABLE EventAdministrator (
id integer PRIMARY KEY,
administratorName text NOT NULL,
title text NOT NULL,
subTitle text NOT NULL,
contactInfo text NOT NULL,
eventId integer,
CONSTRAINT fk_Event
FOREIGN KEY (eventId)
REFERENCES Event(id)
ON DELETE CASCADE);
CREATE TABLE EventAdvertisement (
id integer PRIMARY KEY,
title text NOT NULL,
subTitle text NOT NULL,
adImagePath text NOT NULL,
contactInfo text NOT NULL,
eventId integer,
CONSTRAINT fk_Event
FOREIGN KEY (eventId)
REFERENCES Event(id)
ON DELETE CASCADE);
CREATE TABLE ArtificialIntelligence(
id integer PRIMARY KEY,
title text NOT NULL,
description text NOT NULL,
algorithmPath text,
dataSourcePath text,
dataTargetPath text);
CREATE TABLE MachineLearning(
id integer PRIMARY KEY,
title text NOT NULL,
description text NOT NULL,
algorithmPath text,
dataSourcePath text,
dataTargetPath text);
CREATE TABLE AccountUser (
id integer PRIMARY KEY,
username TEXT NOT NULL,
firstName TEXT NOT NULL,
lastName TEXT NOT NULL,
email TEXT NOT NULL,
phoneNumber TEXT,
address1 TEXT,
address2 TEXT,
city TEXT,
statecode TEXT,
zipcode TEXT,
businessName TEXT,
websiteName TEXT);
CREATE TABLE AccountUserDetails (
id integer PRIMARY KEY,
content1 TEXT NOT NULL,
content2 TEXT NOT NULL,
content3 TEXT NOT NULL,
accountUserId integer NOT NULL,
FOREIGN KEY (accountUserId)
REFERENCES AccountUser (id)
ON UPDATE CASCADE
ON DELETE CASCADE
);
CREATE TABLE Game (
id integer PRIMARY KEY,
title text NOT NULL,
highScore integer NOT NULL,
created datetime DEFAULT CURRENT_TIMESTAMP,
lastUpdated datetime DEFAULT CURRENT_TIMESTAMP);
INSERT INTO Game VALUES(1,'blocks',25,1608860713935,1608860713936);
INSERT INTO Game VALUES(2,'disco trucks',15,1608861793474,1608861793474);
INSERT INTO Game VALUES(3,'ghosts',0,'2020-11-16 09:00:00','2020-11-16 09:00:00');
INSERT INTO Game VALUES(4,'pastel blocks',0,'2020-11-16 09:00:00','2020-11-16 09:00:00');
INSERT INTO Game VALUES(5,'rhythm blocks',0,'2020-11-16 09:00:00','2020-11-16 09:00:00');
INSERT INTO Game VALUES(6,'tank blocks',0,'2020-11-16 09:00:00','2020-11-16 09:00:00');
CREATE TABLE ShopMessage (
id INTEGER PRIMARY KEY,
message TEXT,
email TEXT,
body TEXT,
createdTime DATETIME DEFAULT CURRENT_TIMESTAMP
);
CREATE TABLE ShopOrderStatus (
id INTEGER PRIMARY KEY,
description TEXT
);
CREATE TABLE ShopPaymentType (
id INTEGER PRIMARY KEY,
paymentTypeDescription TEXT
);
CREATE TABLE ShopProduct(
id INTEGER PRIMARY KEY,
description TEXT,
price FLOAT
);
CREATE TABLE ShopCart
(id integer PRIMARY KEY,
customerId INTEGER,
paymentId INTEGER,
lastModified DATETIME DEFAULT CURRENT_TIMESTAMP,
datePurchased DATETIME DEFAULT CURRENT_TIMESTAMP,
cartStatus INTEGER,
cartDateFinished DATETIME DEFAULT CURRENT_TIMESTAMP,
currency TEXT,
currencyValue FLOAT,
FOREIGN KEY (customerId) REFERENCES AccountUser(id)
);
CREATE TABLE CartItem
(id INTEGER PRIMARY KEY,
cartId INTEGER,
itemId INTEGER,
FOREIGN KEY (cartId) REFERENCES ShopCart(id),
FOREIGN KEY (itemId) REFERENCES ShopItem(id)
);
CREATE TABLE CartStatus (
id INTEGER PRIMARY KEY,
description TEXT);
CREATE TABLE ShopItem (
id INTEGER PRIMARY KEY,
productId INTEGER,
quantity INTEGER,
FOREIGN KEY (productId) REFERENCES ShopProduct(id)
);
CREATE TABLE ShopOrder (
id INTEGER PRIMARY KEY,
customerId INTEGER,
lastModified DATETIME DEFAULT CURRENT_TIMESTAMP,
datePurchased DATETIME DEFAULT CURRENT_TIMESTAMP,
orderAmount FLOAT,
FOREIGN KEY (customerId) REFERENCES AccountUser(id)
);
CREATE TABLE ShopOrderCart (
id integer PRIMARY KEY,
orderId integer,
cartId integer,
orderPrice FLOAT NOT NULL DEFAULT 0.00,
orderTax FLOAT NOT NULL DEFAULT 0.00,
FOREIGN KEY (orderId) REFERENCES ShopOrder(id)
);
CREATE TABLE ShopPayment (
id integer PRIMARY KEY,
paymentType integer,
ccType TEXT,
ccOwner TEXT,
ccNumber TEXT,
ccExpires TEXT,
lastModified DATETIME DEFAULT CURRENT_TIMESTAMP,
datePurchased DATETIME DEFAULT CURRENT_TIMESTAMP,
orderStatus integer NOT NULL DEFAULT 0,
orderDateFinished DATETIME DEFAULT CURRENT_TIMESTAMP,
currency TEXT,
currencyValue FLOAT,
FOREIGN KEY (paymentType) REFERENCES ShopPaymentType(id)
);
CREATE TABLE Blog (
 id integer PRIMARY KEY,
 author text NOT NULL,
 authorDate text NOT NULL,
 title text NOT NULL,
 description text NOT NULL,
 content text NOT NULL);
CREATE TABLE Configuration (
id INTEGER PRIMARY KEY,
messages TEXT NOT NULL,
views INTEGER NOT NULL,
shares INTEGER NOT NULL,
users INTEGER NOT NULL,
dateTime TEXT NOT NULL
);
CREATE TABLE Social (
id INTEGER PRIMARY KEY,
postname TEXT NOT NULL,
content TEXT NOT NULL,
reviewed INTEGER NOT NULL,
published TEXT NOT NULL
);
COMMIT;
