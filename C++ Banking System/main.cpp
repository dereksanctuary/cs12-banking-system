#include <iostream>
#include <vector>
#include <string>

using namespace std;

// base abstract class representing a bank account
class bankAccount {
protected:
    string accountHolderName;
    int accountNumber;
    double balance;
    static int nextAccountNumber;

    void adjustBalance(double amount) {
        balance += amount;
    }

public:
    bankAccount(string name, double initBalance = 0.0) {
        accountHolderName = name;
        balance = initBalance;
        accountNumber = nextAccountNumber++;
    }

    virtual ~bankAccount() {}

    double getBalance() const {
        return balance;
    }

    void deposit(double amount) {
        if (amount > 0) {
            balance += amount;
            cout << "deposit successful: $" << amount << " added.\n";
        } else {
            cout << "invalid deposit amount.\n";
        }
    }

    virtual bool withdraw(double amount) = 0;
    virtual void printAccountInfo() const = 0;
};

int bankAccount::nextAccountNumber = 1000;

// abstract class representing a checking account
class checkingAccount : public bankAccount {
public:
    checkingAccount(string name, double initBalance)
        : bankAccount(name, initBalance) {}
    virtual void writeCheck(double amount) = 0;
    virtual void postInterest() = 0;
};

class serviceChargeChecking : public checkingAccount {
private:
    double serviceCharge;
    int checkLimit;
    int checksUsed;
public:
    serviceChargeChecking(string name, double initBalance, double fee, int maxChecks)
        : checkingAccount(name, initBalance), serviceCharge(fee), checkLimit(maxChecks), checksUsed(0) {}

    void writeCheck(double amount) override {
        if (checksUsed < checkLimit && amount <= getBalance()) {
            adjustBalance(-amount);
            checksUsed++;
            cout << "check for $" << amount << " processed.\n";
        } else {
            cout << "check limit reached or insufficient funds.\n";
        }
    }

    void postInterest() override {}

    bool withdraw(double amount) override {
        if (amount > 0 && amount <= getBalance()) {
            adjustBalance(-amount);
            cout << "withdrawal successful.\n";
            return true;
        }
        cout << "insufficient balance.\n";
        return false;
    }

    void printAccountInfo() const override {
        cout << "service charge checking account: \n";
        cout << "balance: $" << getBalance() << "\n";
    }
};

class noServiceChargeChecking : public checkingAccount {
private:
    double minimumBalance;
    double interestRate;
    double lowerInterestRate;
    double penaltyFee;
public:
    noServiceChargeChecking(string name, double initBalance, double minBalance, double rate, double lowRate, double penalty)
        : checkingAccount(name, initBalance), minimumBalance(minBalance), interestRate(rate), lowerInterestRate(lowRate), penaltyFee(penalty) {}

    void writeCheck(double amount) override {
        if (amount > 0 && amount <= getBalance()) {
            adjustBalance(-amount);
            cout << "check for $" << amount << " processed.\n";
            if (getBalance() < minimumBalance) {
                interestRate = lowerInterestRate;
                cout << "your balance fell below $" << minimumBalance << ". interest rate reduced to " << (lowerInterestRate * 100) << "%.\n";
                adjustBalance(-penaltyFee);
                cout << "a $" << penaltyFee << " penalty fee has been applied.\n";
            }
        } else {
            cout << "insufficient funds for check.\n";
        }
    }

    void postInterest() override {
        double rate = (getBalance() >= minimumBalance) ? interestRate : lowerInterestRate;
        double interest = getBalance() * rate;
        deposit(interest);
        cout << "interest posted at " << (rate * 100) << "% rate.\n";
    }

    bool withdraw(double amount) override {
        if (amount > 0 && amount <= getBalance()) {
            adjustBalance(-amount);
            return true;
        }
        return false;
    }

    void printAccountInfo() const override {
        cout << "no service charge checking account: \n";
        cout << "balance: $" << getBalance() << "\n";
    }
};

class highInterestChecking : public noServiceChargeChecking {
public:
    highInterestChecking(string name, double initBalance, double minBalance, double highRate, double lowRate, double penalty)
        : noServiceChargeChecking(name, initBalance, minBalance, highRate, lowRate, penalty) {}
};

class savingsAccount : public bankAccount {
protected:
    double interestRate;
public:
    savingsAccount(string name, double initBalance, double rate)
        : bankAccount(name, initBalance), interestRate(rate) {}

    void applyInterest() {
        double interest = getBalance() * interestRate;
        deposit(interest);
    }

    bool withdraw(double amount) override {
        if (amount > 0 && amount <= getBalance()) {
            adjustBalance(-amount);
            return true;
        }
        return false;
    }

    void printAccountInfo() const override {
        cout << "savings account: \n";
        cout << "balance: $" << getBalance() << "\n";
    }
};

class highInterestSavings : public savingsAccount {
private:
    double minimumBalance;
    int maxWithdrawals;
    int withdrawalsMade;
public:
    highInterestSavings(string name, double initBalance, double rate, double minBalance, int maxWithdraw)
        : savingsAccount(name, initBalance, rate), minimumBalance(minBalance), maxWithdrawals(maxWithdraw), withdrawalsMade(0) {}

    bool withdraw(double amount) override {
        if (withdrawalsMade >= maxWithdrawals) {
            cout << "withdrawal limit reached!\n";
            return false;
        }

        if (amount > 0 && amount <= getBalance()) {
            adjustBalance(-amount);
            withdrawalsMade++;
            return true;
        }
        return false;
    }

    void printAccountInfo() const override {
        cout << "high interest savings account: \n";
        cout << "balance: $" << getBalance() << "\n";
    }
};

class certificateOfDeposit : public bankAccount {
private:
    int maturityMonths;
    double interestRate;
    double penaltyRate;
public:
    certificateOfDeposit(string name, double initBalance, int months, double rate, double penalty)
        : bankAccount(name, initBalance), maturityMonths(months), interestRate(rate), penaltyRate(penalty) {}

    void applyInterest() {
        if (maturityMonths == 0) {
            double interest = getBalance() * interestRate;
            deposit(interest);
        }
    }

    bool withdraw(double amount) override {
        if (maturityMonths > 0) {
            double penalty = amount * penaltyRate;
            adjustBalance(-penalty);
            cout << "early withdrawal penalty applied: $" << penalty << "\n";
        }
        if (amount <= getBalance()) {
            adjustBalance(-amount);
            return true;
        }
        return false;
    }

    void printAccountInfo() const override {
        cout << "certificate of deposit: \n";
        cout << "balance: $" << getBalance() << "\n";
    }

    void setMaturity(int months) {
        maturityMonths = months;
    }
};

void showTransactionMenu(bankAccount* acc) {
    int action;
    do {
        cout << "\nselect transaction:\n";
        cout << "1. deposit\n2. withdraw\n3. print info\n4. write check (if applicable)\n5. post/apply interest\n6. back to main menu\n";
        cout << "enter choice: ";
        cin >> action;

        if (action == 1) {
            double amt;
            cout << "enter deposit amount: ";
            cin >> amt;
            acc->deposit(amt);
        } else if (action == 2) {
            double amt;
            cout << "enter withdrawal amount: ";
            cin >> amt;
            acc->withdraw(amt);
        } else if (action == 3) {
            acc->printAccountInfo();
        } else if (action == 4) {
            checkingAccount* chk = dynamic_cast<checkingAccount*>(acc);
            if (chk) {
                double amt;
                cout << "enter check amount: ";
                cin >> amt;
                chk->writeCheck(amt);
            } else {
                cout << "this account does not support checks.\n";
            }
        } else if (action == 5) {
            checkingAccount* chk = dynamic_cast<checkingAccount*>(acc);
            if (chk) chk->postInterest();
            else {
                savingsAccount* sav = dynamic_cast<savingsAccount*>(acc);
                if (sav) sav->applyInterest();
                else {
                    certificateOfDeposit* cd = dynamic_cast<certificateOfDeposit*>(acc);
                    if (cd) {
                        cd->setMaturity(0); // force maturity so interest applies
                        cd->applyInterest();
                    } else cout << "this account cannot apply interest.\n";
                }
            }
        }
    } while (action != 6);
}

int main() {
    vector<bankAccount*> accounts;
    int choice;

    do {
        cout << "\n--- bank account management ---\n";
        cout << "1. open a new account\n";
        cout << "2. view account details\n";
        cout << "3. perform transactions\n";
        cout << "4. exit\n";
        cout << "enter choice: ";
        cin >> choice;

        if (choice == 1) {
            string name;
            double initialDeposit;
            int accountType;

            cout << "enter your name: ";
            cin.ignore();
            getline(cin, name);

            cout << "enter initial deposit amount: $";
            cin >> initialDeposit;

            cout << "select account type:\n";
            cout << "1. service charge checking\n";
            cout << "2. no service charge checking\n";
            cout << "3. high interest checking\n";
            cout << "4. regular savings\n";
            cout << "5. high interest savings\n";
            cout << "6. certificate of deposit (cd)\n";
            cout << "enter choice: ";
            cin >> accountType;

            if (accountType == 1) {
                accounts.push_back(new serviceChargeChecking(name, initialDeposit, 10.0, 3));
            } else if (accountType == 2) {
                accounts.push_back(new noServiceChargeChecking(name, initialDeposit, 500, 0.02, 0.01, 10));
            } else if (accountType == 3) {
                accounts.push_back(new highInterestChecking(name, initialDeposit, 1000, 0.05, 0.01, 10));
            } else if (accountType == 4) {
                accounts.push_back(new savingsAccount(name, initialDeposit, 0.02));
            } else if (accountType == 5) {
                accounts.push_back(new highInterestSavings(name, initialDeposit, 0.04, 1000, 3));
            } else if (accountType == 6) {
                accounts.push_back(new certificateOfDeposit(name, initialDeposit, 6, 0.05, 0.01));
            }
        } else if (choice == 2) {
            for (bankAccount* acc : accounts) {
                acc->printAccountInfo();
                cout << "\n";
            }
        } else if (choice == 3) {
            int index;
            cout << "enter account index (starting from 0): ";
            cin >> index;
            if (index >= 0 && index < accounts.size()) {
                showTransactionMenu(accounts[index]);
            } else {
                cout << "invalid index.\n";
            }
        }
    } while (choice != 4);

    for (bankAccount* acc : accounts) {
        delete acc;
    }

    cout << "exiting program. have a gucci day!\n";
    return 0;
}
