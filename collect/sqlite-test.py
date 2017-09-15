#!/usr/bin/python

import sys
import random
import string

import sqlite3

def fake_Word(a, b):
    return random.choice(string.uppercase) + "".join([random.choice(string.lowercase) for _ in range(random.randint(a-1, b-1))])

def fake_name():
    return fake_Word(3, 7) + " " + fake_Word(4, 12)

def fake_address():
    return str(random.randint(1, 1000)) + " " + fake_Word(3, 15) + " " + \
        random.choice(["St.", "Dr.", "Sq.", "Rd.", "Ave", "Way", "Ln", "Blvd.", "Jct."]) + " " + \
        str(random.randint(10000, 99999)) + " " + \
        random.choice(string.uppercase) + random.choice(string.uppercase) + " " + \
        fake_Word(6, 9)

class Database:
    numberOfEntries = 10000000

    def __init__(self, filename):
        self.conn = sqlite3.connect(filename)
        self.nrQueries = 0
        self.nrDeposits = 0
        self.nrAddressChanges = 0

    def create(self):
        self.conn.isolation_level = None
        c = self.conn.cursor()
        c.execute("CREATE TABLE ledger (accountId INTEGER PRIMARY KEY, name TEXT, address TEXT, money INTEGER)")
        c.execute("BEGIN")
        for _ in range(numberOfEntries):
            c.execute("INSERT INTO ledger (name,address,money) VALUES(?,?,?)",
                      (fake_name(), fake_address(), random.randint(50000,100000)))
        c.execute("COMMIT")

    def runTest(self, doAppCache):
        self.conn.isolation_level = None
        c = self.conn.cursor()
        #
        for _a in range(500):
            c.execute("BEGIN")
            for _b in range(10000):
                op = random.randint(0,100)
                if op < 90:
                    self.nrQueries += 1
                    c.execute("SELECT * FROM ledger WHERE accountId=?", (random.randint(1, self.numberOfEntries-1),))
                elif op < 97:
                    self.nrDeposits += 1
                    c.execute("UPDATE ledger SET money=money+? WHERE accountId=?",
                              (random.randint(-10,10), random.randint(1, self.numberOfEntries-1)))
                else:
                    self.nrAddressChanges += 1
                    c.execute("UPDATE ledger SET address=? WHERE accountId=?",
                              (fake_address(), random.randint(1, self.numberOfEntries-1)))
            c.execute("COMMIT")
            print _a
        #
        print "nrQueries = ", self.nrQueries
        print "nrDeposits = ", self.nrDeposits
        print "nrAddressChanges = ", self.nrAddressChanges

    def close(self):
        self.conn.commit()
        self.conn.close()
        self.conn = None


if sys.argv[1] == "create":
    database = Database(sys.argv[2])
    database.create()
    database.close()
elif sys.argv[1] == "run":
    database = Database(sys.argv[2])
    database.runTest(False)
    database.close()
