# CPP_ParticleSimulation
A C++ Program Which Implements A Real-Time Particle Simulation Through OpenGL For Optimal Collision And Optimized Concurrent Particle Interactions

----------------------------------------------

<img src="https://github.com/user-attachments/assets/b42434d4-bd8f-46c2-b5e7-5de18d4db46b" alt="Cornstarch <3" width="75" height="99"> <img src="https://github.com/user-attachments/assets/b42434d4-bd8f-46c2-b5e7-5de18d4db46b" alt="Cornstarch <3" width="75" height="99"> <img src="https://github.com/user-attachments/assets/b42434d4-bd8f-46c2-b5e7-5de18d4db46b" alt="Cornstarch <3" width="75" height="99"> <img src="https://github.com/user-attachments/assets/b42434d4-bd8f-46c2-b5e7-5de18d4db46b" alt="Cornstarch <3" width="75" height="99"> 


<h3>**The Breakdown:**</h3>

This Program Has 2 Main Scripts To Be Ran, Starting With A C++ Process Which Will Benchmark 2 Differing Implementations To Solve The 0-1 Knapsack Problem:
  <br>&nbsp; 1) Recursive Optimal Exhaustive Include-Exclude Algorithm: A O(2^n) Runtime Complexity Algorithm Which Checks All Subset Combinations
  <br>&nbsp; 2) Iterative Suboptimal "Standard Greedy" (Profit-Per-Weight) Heuristic: A O(n*log(n)) Runtime Complexity Heuristic Binning By Profit-Per-Weight

Then A Python Script Which Generates Charts And Graphs Based Upon The Benchmark Results Which Are Stored In a .json File.

The Program Is Meant To See How Varying Algorthmic Implementations Increases In Runtime Complexity And Considerations For Each Of Their Utility Depending On Domain-Knowledge. 

For 0-1 Knapsack Problem It Usually Utilizes 2 Arrays For weight And profit Of Each Item But Without names This Is A Custom Struct That Holds weight, profit, And name For Each Item For Easier Debugging And Unit Testing For Plotting Of Increasing n:

    struct Item
    {
      float weight = 0.0f;
      float profit = 0.0f;
      std::string name;
    };


After The Program Is Ran, It Will Run 45 Individual Benchmarks On The Two Differing Algorithms--With Both Utilizing The Same Items To Consider To Ensure Equal Argument Complexity--Each Of These Tests Will Be Done In Increments Of 10 And Be Tracked Using std::chrono In Milliseconds; This Means We Are Running From 10 -> 450. This Allows Us To See How Runtime Scales Between Varying Values Of n, Where n Is The Number Of Input Items We Are Checking To Insert.

Each Test Will Utilize This Code Snippet To Generate Our Items For Each Increment Of n:

    Item* items = new Item[n];
    for(unsigned int i = 0; i < n; i++)
    {
        items[i] =
        {
            static_cast<float>(i + 1),
            static_cast<float>((i + 1) * 10),
            "Item" + std::to_string(i)
        };
    }

While Our Generation Is Pattern-Generated And Quite Primitive It Still Works As Great Means Of Conducting Analysis Tests As It Allows More Replicable Results Between Systems And Easier Scalability. This Is The Same For Our Max capacity Of Our Knapsack Bins Which Scales With Our n:

    float capacity = static_cast<float>(n) * 0.5f;

After Each Test, The Results Will Be Placed In A .json File In The Local Directory Under _**"benchmark_results.json"**_. Each Entry Will Be Inserted In The (x,y) Coordinate Planed To Later Be Graphed With n As Our x-axis And y-axis As Our Runtime. The .json For Each Entry Will Generate A Name For This Individual (x,y) Coordinate As Seen In The Loop:

    std::ofstream jsonFile("benchmark_results.json");
    jsonFile << "{\n    \"Benchmark Results\": {\n";

    for(size_t i = 0; i < results.size(); i++)
    {
        jsonFile << "\"Test " << results[i].n << "\": {\n";
        jsonFile << "    \"n\": " << results[i].n << ",\n";
        jsonFile << "    \"y\": " << results[i].runtime << "\n";
        jsonFile << "        }";
        if(i < results.size() - 1) jsonFile << ",";
        jsonFile << "\n";
    }

    jsonFile << "    }\n}";
    jsonFile.close();

After The C++ Benchmarks Are Done You Can Run The Python Script Which Generates Multiple Charts And Graphs Based Upon The Entries For Comparison Of The Two Implementations With Some Charts Being Just A Graph Of The Data For A Single Implmentation And Others Being Overlap Of Both Them. Our Results Also Displayed The Exhaustive Algorithm Needing To Be Charted With The y-axis Being In Hours And The Heuristic Needing To Be Charted In Microseconds.

Utilizing The .json Data And Python Charts Created, There Is A Created White Paper On My Findings Between These Two Implenentations, Their Unique Pros-And-Cons As Well As Domain-Applicability As Each Has Their Usage Mainly With One Being Optimal And The Other Being Sub-Optimal.

<img src="https://github.com/user-attachments/assets/00f8d76b-9e49-432c-9506-3d460840a991" alt="Cornstarch <3" width="75" height="99"> <img src="https://github.com/user-attachments/assets/00f8d76b-9e49-432c-9506-3d460840a991" alt="Cornstarch <3" width="75" height="99"> <img src="https://github.com/user-attachments/assets/00f8d76b-9e49-432c-9506-3d460840a991" alt="Cornstarch <3" width="75" height="99"> <img src="https://github.com/user-attachments/assets/00f8d76b-9e49-432c-9506-3d460840a991" alt="Cornstarch <3" width="75" height="99"> 

----------------------------------------------

<img src="https://github.com/user-attachments/assets/645e1b72-7232-4f0c-9214-ec72adf171cf" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/645e1b72-7232-4f0c-9214-ec72adf171cf" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/645e1b72-7232-4f0c-9214-ec72adf171cf" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/645e1b72-7232-4f0c-9214-ec72adf171cf" alt="Cornstarch <3" width="55" height="49">  




<h3>**Features:**</h3>

Generated Benchmark Charts From Python Script Using Collected Data:
![algorithm_comparison](https://github.com/user-attachments/assets/d6297d24-06dd-43b3-ba97-4cccfdc59476)

![knapsack_algorithm_comparison_split](https://github.com/user-attachments/assets/28fefa31-6856-4a5e-b491-dc36a61b6550)

![low_n_comparison](https://github.com/user-attachments/assets/379f939f-4d5b-4bee-b062-bd0090fdff19)

![runtime_analysis_heuristic](https://github.com/user-attachments/assets/840c5d62-99a5-4075-a423-1fea90fbbf82)

![runtime_analysis](https://github.com/user-attachments/assets/2973feea-065f-4349-8feb-7fcd90b3d456)


<img src="https://github.com/user-attachments/assets/db3f3910-93d1-4bb7-9e37-afffe205566f" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/db3f3910-93d1-4bb7-9e37-afffe205566f" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/db3f3910-93d1-4bb7-9e37-afffe205566f" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/db3f3910-93d1-4bb7-9e37-afffe205566f" alt="Cornstarch <3" width="55" height="49">  


