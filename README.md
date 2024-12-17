# CPP_ParticleSimulation
This Project Works With OpenGL To Create A Visual Real-Time Particle Simulation With Their Own Collision Detection To Help See How We Can Optimize Means Of Collision Detection Going From A Brute-Force Runtime Complexity For The Collision Algorithm Of O(n!) To A O(n) Complexity By Utilizing Radial Sweeps/Checks On Neighboring Particles Which Are Apportioned Into HashMap Cells Using A Custom Hash Mapping Algorithm To Allow For O(1) Insertions/Removals.

While Currently Not At Its Final Stable Build, It Is Still Very Functional And Has Been Showing Smooth 60fps Even With 100s Of Particles Concurrently Being Simulated And Collided. Currently Working On This Project As Means Of Learning About Realistic Implementations For Algorithmic Complexity Analysis And How Values Of n Can Heavily Bias Runtime Efficiency. While I've Been Comfortable And Knowledgable About Varying Means Of Solving Problems Optimally, This Project And My Courses Have Helped Me Understand How To Think More Theoretically Instead Of More Pragmatically About Problems; Allowing Me To Reason Through Means Of Using HashMaps And Other Standardized Data Structures For Optimal And Practical Applications Mainly In The Sector Of Simulations. 

Interesting Lessons Have Been Learned In This Project With Multithreading Contention And Means Of Cleanly Partitioning Regions To Allow For Non-Conflicting--And In Turn Non-Synchronizing Regions--To Be Employed For Intuitive And Scalable Solutions.

While The Collision Detection Currently Is Quite Primitive (Mostly With The Screen Edges As Theres Cases Where Particles Clip Out Of The Visual Region) It Still Has Worked Great And Will Be Iterated On In Future Patches For More Dynamic And Stable Detection Algorithms. If Two Particles Currently Collide, They Don't Have Rigid Bodies And Have A Binding Impulse Force Which Will Try To Stick Them Together If They Collide As Would Particles With Chemical Bonding (Albeit Very Primitive Currently). Also The Codebase Mainly Resides In The Driver.cpp Which Is Kind Of Party-Foul But Will Be Cleaned Up In Future Patches.

You Can Also Click To Create A Force-Pulse Which Is Basically A Circular Region From Your Cursor Which Will Push Away All Particles In The Circles Radius If You Left Click, And Pull In All Particles In The Circles Radius If You Right Click.

----------------------------------------------

<img src="https://github.com/user-attachments/assets/4d236ec7-91a8-4128-851b-db5c476a7086" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/4d236ec7-91a8-4128-851b-db5c476a7086" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/4d236ec7-91a8-4128-851b-db5c476a7086" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/4d236ec7-91a8-4128-851b-db5c476a7086" alt="Cornstarch <3" width="55" height="49"> 

<h3>**The Breakdown:**</h3>



<img src="https://github.com/user-attachments/assets/0c481edf-693f-4b4b-bca2-f6017a3e15d4" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/0c481edf-693f-4b4b-bca2-f6017a3e15d4" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/0c481edf-693f-4b4b-bca2-f6017a3e15d4" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/0c481edf-693f-4b4b-bca2-f6017a3e15d4" alt="Cornstarch <3" width="55" height="49"> 

----------------------------------------------

<img src="https://github.com/user-attachments/assets/69234949-e52b-4056-9b5b-382fa1f28745" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/69234949-e52b-4056-9b5b-382fa1f28745" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/69234949-e52b-4056-9b5b-382fa1f28745" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/69234949-e52b-4056-9b5b-382fa1f28745" alt="Cornstarch <3" width="55" height="49"> 

<h3>**Features:**</h3>

Generated Benchmark Charts From Python Script Using Collected Data:
![algorithm_comparison](https://github.com/user-attachments/assets/d6297d24-06dd-43b3-ba97-4cccfdc59476)

![knapsack_algorithm_comparison_split](https://github.com/user-attachments/assets/28fefa31-6856-4a5e-b491-dc36a61b6550)

![low_n_comparison](https://github.com/user-attachments/assets/379f939f-4d5b-4bee-b062-bd0090fdff19)

![runtime_analysis_heuristic](https://github.com/user-attachments/assets/840c5d62-99a5-4075-a423-1fea90fbbf82)

![runtime_analysis](https://github.com/user-attachments/assets/2973feea-065f-4349-8feb-7fcd90b3d456)


<img src="https://github.com/user-attachments/assets/0363619c-11fb-472f-b338-39b361304dd5" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/0363619c-11fb-472f-b338-39b361304dd5" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/0363619c-11fb-472f-b338-39b361304dd5" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/0363619c-11fb-472f-b338-39b361304dd5" alt="Cornstarch <3" width="55" height="49"> 

