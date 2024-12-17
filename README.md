# CPP_ParticleSimulation
This Project Works With OpenGL To Create A Visual Real-Time Particle Simulation With Their Own Collision Detection To Help See How We Can Optimize Means Of Collision Detection Going From A Brute-Force Runtime Complexity For The Collision Algorithm Of O(n!) To A O(n) Complexity By Utilizing Radial Sweeps/Checks On Neighboring Particles Which Are Apportioned Into HashMap Cells Using A Custom Hash Mapping Algorithm To Allow For O(1) Insertions/Removals.

While Currently Not At Its Final Stable Build, It Is Still Very Functional And Has Been Showing Smooth 60fps Even With 100s Of Particles Concurrently Being Simulated And Collided. Currently Working On This Project As Means Of Learning About Realistic Implementations For Algorithmic Complexity Analysis And How Values Of n Can Heavily Bias Runtime Efficiency. While I've Been Comfortable And Knowledgable About Varying Means Of Solving Problems Optimally, This Project And My Courses Have Helped Me Understand How To Think More Theoretically Instead Of More Pragmatically About Problems; Allowing Me To Reason Through Means Of Using HashMaps And Other Standardized Data Structures For Optimal And Practical Applications Mainly In The Sector Of Simulations. 

Interesting Lessons Have Been Learned In This Project With Multithreading Contention And Means Of Cleanly Partitioning Regions To Allow For Non-Conflicting--And In Turn Non-Synchronizing Regions--To Be Employed For Intuitive And Scalable Solutions.

While The Collision Detection Currently Is Quite Primitive (Mostly With The Screen Edges As Theres Cases Where Particles Clip Out Of The Visual Region) It Still Has Worked Great And Will Be Iterated On In Future Patches For More Dynamic And Stable Detection Algorithms. If Two Particles Currently Collide, They Don't Have Rigid Bodies And Have A Binding Impulse Force Which Will Try To Stick Them Together If They Collide As Would Particles With Chemical Bonding (Albeit Very Primitive Currently). Also The Codebase Mainly Resides In The Driver.cpp Which Is Kind Of Party-Foul But Will Be Cleaned Up In Future Patches.

You Can Also Click To Create A Force-Pulse Which Is Basically A Circular Region From Your Cursor Which Will Push Away All Particles In The Circles Radius If You Left Click, And Pull In All Particles In The Circles Radius If You Right Click.

----------------------------------------------

<img src="https://github.com/user-attachments/assets/4d236ec7-91a8-4128-851b-db5c476a7086" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/4d236ec7-91a8-4128-851b-db5c476a7086" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/4d236ec7-91a8-4128-851b-db5c476a7086" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/4d236ec7-91a8-4128-851b-db5c476a7086" alt="Cornstarch <3" width="55" height="49"> 

<h3>**The Breakdown:**</h3>

The Program Starts By Initializing A Window Based Upon The Custom ParticleWindow Class. This Class Works As Means Of Encapsulating All Data Pertaining To The Simulation Including A GLFW Window, The Shader For This Provided Window, As Well As Data Pertaining To Rendering And The Current Particles Being Simulated.

When The Process Initializes The Class, It Starts By Contexualizing The Window For OpenGL Buffer Contexting As Well As Generating Our Particles Through _**initParticles(...)**_.

In _**initParticles(...)**_ We Utilize Random Generation To Get Varying Particles From Their Color, Initial Position, Initial Velocity, As Well As Their Radial Size. For This Current Project, Instead Of Utilizing An Array-Of-Structs (AoS [Array Of Particle Instances]) We Utilize An Struct-Of-Arrays (SoA [Instance Of Particle Traits Arrays]) To Decrease Cache Thrashing And Increase Locality Of Shared Data Which We Utilize In Our Simulation As Ensuring Data Like Positions Are All Accessed And Checked Together In The Cache Is Better Than Having Multiple Particles And All Their Data Being In The Cache Basically One-At-A-Time. This Is A Trade Off As While More Efficient It Is A Little Less Readable And Intuitive As Individual Particles Are Represented Through Indexes Instead Of Class Instances. After Each Particle Has Its Provided Backend Members Initialized (Centriod, Radii, Color, Velocity, ID), We Start Building The Circles Visual Elements Through A Circle Algorithm Which Generates All Vertexes For The VBO (Vertex Array Buffer) And Vertices For The EBO (Element Array Buffer). These Two Entries Allow OpenGL To Properly Render Our Particles.

After Generation Is Done In _**initParticles(...)**_ We Then Create Threads Which Allows Us To Partition The Workload For Collision Detection To Multiple "Workers" To Allow Us To Gain Runtime Benefits As Instead Of One Thread Sequentially Updating All Particles Itself It Divides The Workload To Multiple Threads (Two Threads Currently As From Testing It Seemed Two Allowed Runtime Benefits Without Diminishing Returns From Context Swapping). These Threads Are Synchronized With The Main Window Thread Through The _**updateBarrier**_ & _**renderBarrier**_ Barriers To Ensure Workers Render For Each Frame And Don't Run Ahead Frames Which Could Cause Artifacts In Our Simulation. 

Each Thread Does The Motion & Collision Detection For Particles It Is Delegated. This Delegation Is Done By Providing Each Thread With A Start Position In The SoA Starting At Index 0 And Going Up Based On Its Provided _**thread_id**_ (Simply What Thread Was Initialized First, Second, Etc. [I.E. First Thread Initialized Is 0, Second 1]) Then It Increments By The _**NUM_THREADS**_ We Have To Allow It To Jump Across To The Next Thread It Is Delegated, Ensuring Multiple Threads Don't Access The Same Particle (Which Could Lead To Double-Updates Which Could Lead To Crashes Or Unintended Behaivor). Each Particle Will Reflect Off Window Borders By Inverting Its Velocity, And If Colliding With A Particle Will Cause The Binding Impulsive Force On These Two Particles; This Force Is Light To Allow Bonds To Be Broken And To Ensure Particles Don't Snap Together And Is More A Gradual Attraction Towards Eachother.

We Then Start On The ParticleWindow's _**run(...)**_ Function In The Main Thread Which Is The Main Rendering Loop. This Is A Simple Rendering Loop With A Target Frame Rate To Reach And Simple Buffer Clearing And Swapping. Other Than This, We Have The _**addParticle(...)**_ Function Which For Each Frame And Each Particle Will Emplace Them In The collisionMatrix (Our HashMap). We Will Then Arrive At Our _**updateBarrier**_ Which Will Tell Our Worker Threads To Do Their Collision And Motion Logic As We Wait At The _**renderBarrier**_ Which Will Push Us Forward After Each Worker Has Finalized Their Collision And Motion Logic For Us To Then Render The Results In _**renderParticles(...)**_. We Then Clear Our HashMap For The Next Frame As We Will Have To Again Load Our Particles Into The HashMap Using Their New Positions After This Frame.

This Is The Main Loop Of Our Simulation; We Also Include A _**mouseButtonCallbackWrapper(...)**_ Which Will Call Our _**mouseButtonCallback(...)**_ Which Will Check What Type Of Click We Have Done On The Screen And Apply The Provided Repulsive Force To All Particles In That Radius In _**applyRepulsiveForce(...)**_.

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


<img src="https://github.com/user-attachments/assets/0363619c-11fb-472f-b338-39b361304dd5" alt="Cornstarch <3" width="55" height="69"> <img src="https://github.com/user-attachments/assets/0363619c-11fb-472f-b338-39b361304dd5" alt="Cornstarch <3" width="55" height="69"> <img src="https://github.com/user-attachments/assets/0363619c-11fb-472f-b338-39b361304dd5" alt="Cornstarch <3" width="55" height="69"> <img src="https://github.com/user-attachments/assets/0363619c-11fb-472f-b338-39b361304dd5" alt="Cornstarch <3" width="55" height="69">

