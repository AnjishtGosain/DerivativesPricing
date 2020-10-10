# Derivatives Pricing and Optimisation

This code prices equity European and American options using Monte Carlo and Tree implementations of Black Scholes style models (standard, with dividend, and with single and double Normal jumps). This project also contains an implementation of a Differential Evolution algorithm to back-solve model parameters given market data in JSON format. This could be used to calibrate the models.


## Table of contents
* [Features](#features)
* [To-Do List](#to-do-list)
* [Projects and Namespaces](#projects-and-namespaces)
* [Technologies](#technologies)
* [Setup](#setup)
* [Status](#status)
* [Contact](#contact)


## Features
* American option pricing under variations of Black Scholes using tree methods:
    * Standard: with state probabilities from Cox, Ross and Rubinstein (1979) and Tian (1993), Richardson Extrapolation, and Smoothing. 
    * With Dividends: stock prices drops at a pre-determined time by a predetermined amount.
    * Single Normal Jump: Stochastic earnings event, inspired by Hilliard and Schwartz (2005).
    * Double Normal Jump: A further enhancement of the previous, this time with the earnings event being dependent upon two different normals.  
* Differential Evolution solver to back-solve for model parameters. Useful for model calibration.

## To-Do List
* Calculate the Greeks for the options. This is a very important upgrade, as these will be needed for hedging.
* Unit Tests for the individual projects. Only Integration Tests have currently been implemented.
* Add other pricing methods for American options.
* Implement Levenberg-Marquardt as an alternative to Differential Evolution, though this will need to be done after the Greeks have been calculated. 


## Projects and Namespaces

I have implemented my solution in C++11, in the Visual Studio solution DerivativesPricingSolution.sln. Although the solution is designed to be compiled on Windows, the code should be portable to Linux. Other than the C++11 Standard Template Library, the only other package I have used was Boost. 

The solution contains the following Visual Studio Projects/C++ namespaces:

* API: contains the individual function APIs for the exercises (see Methodology_and_Exercises.docx). Each API is a wrapper around classes and functions in the other projects.

* Instruments: contains the derivative payoffs, which in the present case is simply vanilla options.

* Models: the different models such as the Black Scholes, Black Scholes with Normal Jump etc. These classes describe the dynamics of the underlying asset price, and simulate values for use by the Pricers.

* Pricers: these are the model implementations used to price the derivatives. For example, the namespace contains tree, Monte Carlo, and analytic pricing methods for vanilla options. Pricers and Models are linked together via the Delegate design pattern. Each Pricer is set up to work with a unique pure virtual class of model, which is then inherited by the actual models in the Models project.

* Optimisers: contains the implementation of the Differential Evolution optimiser. The optimiser requires input of a function object, parameter constraints, and convergence criterion. It returns the parameters which minimise the function. The optimiser has been designed to minimise any continuous function, not just the ones from the Pricers project.

* TestHarness: this project contains a simple test harness and integration tests for the validation of the tree Pricer and Differential Evolution optimiser.

* Enumerations: avoids the use of strings for the identification of commonly used option features such as exercise type, option right, underlying asset codes etc.

All random generation was done using the Standard Template Library implementation of the Mersenne Twister pseudo random generation algorithm.


## Technologies
This project was created in Visual Studio, using SDK 10.0.16299.0. The C++ libraries used were:

* C++11 Standard Template Library
* Boost 1.66


## Setup
Follow the below steps to run the project in Visual Studio:

1. Open the “API” Visual Studio Solution in Visual Studio.
2. From Solution Explorer, right click on “API”, and select “Set as Start Up Project”.
3. Under Build, select Configuration Manager, and set the Active solution configuration to Release, and select x64 as the platform. This compiles the solution with compiler optimisation (equivalent to -O2 command line flag) in a 64-bit environment.
4. Hit F5. This will open a command prompt displaying the outputs.

As mentioned in [Technologies](#technologies), this project uses Boost 1.66 (primarily for the optimisation calculations). The steps to install Boost are as follows:

1. Download boost_1_66_0.zip from "http://www.boost.org/users/history/version_1_66_0.html
2. Unzip the downloaded .zip file in C:\Program Files\boost\
All of the Visual Studio projects which are dependent on boost already have C:\Program Files\boost\boost_1_66_0\ as an Include Directory.


## Status
Project is: _in progress_


## Contact
Created by [@AnjishtGosain], https://www.linkedin.com/in/anjisht-gosain-8aa480172/?originalSubdomain=au)



