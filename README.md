# CPP_ParticleSimulation
This Project Works With OpenGL To Create A Visual Real-Time Particle Simulation With Their Own Collision Detection To Help See How We Can Optimize Means Of Collision Detection Going From A Brute-Force Runtime Complexity For The Collision Algorithm Of O(n!) To A O(n) Complexity By Utilizing Radial Sweeps/Checks On Neighboring Particles Which Are Apportioned Into HashMap Cells Using A Custom Hash Mapping Algorithm To Allow For O(1) Insertions/Removals.

While Currently Not At Its Final Stable Build, It Is Still Very Functional And Has Been Showing Smooth 60fps Even With 100s Of Particles Concurrently Being Simulated And Collided. Currently Working On This Project As Means Of Learning About Realistic Implementations For Algorithmic Complexity Analysis And How Values Of n Can Heavily Bias Runtime Efficiency. While I've Been Comfortable And Knowledgable About Varying Means Of Solving Problems Optimally, This Project And My Courses Have Helped Me Understand How To Think More Theoretically Instead Of More Pragmatically About Problems; Allowing Me To Reason Through Means Of Using HashMaps And Other Standardized Data Structures For Optimal And Practical Applications Mainly In The Sector Of Simulations. 

Interesting Lessons Have Been Learned In This Project With Multithreading Contention And Means Of Cleanly Partitioning Regions To Allow For Non-Conflicting--And In Turn Non-Synchronizing Regions--To Be Employed For Intuitive And Scalable Solutions.

----------------------------------------------

<img src="https://github.com/user-attachments/assets/b42434d4-bd8f-46c2-b5e7-5de18d4db46b" alt="Cornstarch <3" width="75" height="99"> <img src="https://github.com/user-attachments/assets/b42434d4-bd8f-46c2-b5e7-5de18d4db46b" alt="Cornstarch <3" width="75" height="99"> <img src="https://github.com/user-attachments/assets/b42434d4-bd8f-46c2-b5e7-5de18d4db46b" alt="Cornstarch <3" width="75" height="99"> <img src="https://github.com/user-attachments/assets/b42434d4-bd8f-46c2-b5e7-5de18d4db46b" alt="Cornstarch <3" width="75" height="99"> 


<h3>**The Breakdown:**</h3>



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


