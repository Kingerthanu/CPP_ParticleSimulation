# CPP_ParticleSimulation
This Project Works With OpenGL To Create A Visual Real-Time Particle Simulation With Their Own Collision Detection To Help See How We Can Optimize Means Of Collision Detection Going From A Brute-Force Runtime Complexity For The Collision Algorithm Of O(n!) To A O(n) Complexity By Utilizing Radial Sweeps/Checks On Neighboring Particles Which Are Apportioned Into HashMap Cells Using A Custom Hash Mapping Algorithm To Allow For O(1) Insertions/Removals.

While Currently Not At Its Final Stable Build, It Is Still Very Functional And Has Been Showing Smooth 90fps Even With 1000s Of Particles Concurrently Being Simulated And Collided. Currently Working On This Project As Means Of Learning About Realistic Implementations For Algorithmic Complexity Analysis And How Values Of n Can Heavily Bias Runtime Efficiency. While I've Been Comfortable And Knowledgable About Varying Means Of Solving Problems Optimally, This Project And My Courses Have Helped Me Understand How To Think More Theoretically Instead Of More Pragmatically About Problems; Allowing Me To Reason Through Means Of Using HashMaps And Other Standardized Data Structures For Optimal And Practical Applications Mainly In The Sector Of Simulations. 

Interesting Lessons Have Been Learned In This Project With Multithreading Contention And Means Of Cleanly Partitioning Regions To Allow For Non-Conflicting--And In Turn Non-Synchronizing Regions--To Be Employed For Intuitive And Scalable Solutions.

While The Collision Detection Currently Is Quite Primitive (Mostly With The Screen Edges As Theres Cases Where Particles Clip Out Of The Visual Region) It Still Has Worked Great And Will Be Iterated On In Future Patches For More Dynamic And Stable Detection Algorithms. If Two Particles Currently Collide, They Don't Have Rigid Bodies And Have A Binding Impulse Force Which Will Try To Stick Them Together If They Collide As Would Particles With Chemical Bonding (Albeit Very Primitive Currently). Also The Codebase Mainly Resides In The Driver.cpp Which Is Kind Of Party-Foul But Will Be Cleaned Up In Future Patches.

You Can Also Click To Create A Force-Pulse Which Is Basically A Circular Region From Your Cursor Which Will Push Away All Particles In The Circles Radius If You Left Click, And Pull In All Particles In The Circles Radius If You Right Click.

----------------------------------------------

<img src="https://github.com/user-attachments/assets/4d236ec7-91a8-4128-851b-db5c476a7086" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/4d236ec7-91a8-4128-851b-db5c476a7086" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/4d236ec7-91a8-4128-851b-db5c476a7086" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/4d236ec7-91a8-4128-851b-db5c476a7086" alt="Cornstarch <3" width="55" height="49"> 

<h3>The Breakdown:</h3>

In This Second Patch, We've Significantly Evolved Our Real-Time Particle Simulation From A Simple Bonding/Attraction Model Into A More Physically Dynamic Scenario Where Particles Repel And Push Away From One Another, Thus More Closely Approximating Non-Rigid-Body Collisions. This Patch Focuses Heavily On Improving The Sense Of Physical Realism: Rather Than Particles "Clumping" And Forming Primitive Bonds As In Patch 1, They Now Behave As If Under Gravitational Influence And Collision Impulses, Dispersing And Scattering Upon Contact.

We Continue To Utilize OpenGL For Rendering And Have Refined Our Internal Data Structures, Spatial Hashing, And Threading Model. While The General Architecture Remains Similar, Numerous Enhancements Have Been Integrated To Boost Performance, Accuracy, And Scalability.

_**Key Differences From Patch 1:**_

* **Collision Dynamics:**
* Patch 1’s Particles Adhered Together Upon Collision, Resulting In A Visual "Bonding." In Patch 2, We Now Implement Collision Responses That Push Particles Apart, Simulating Non-Rigid Impacts More Akin To Realistic Elastic Collisions. This Removes The "Sticky" Interactions And Allows Particles To Scatter And Spread. This Collision Response Is Handled Using The `resolveCollision(...)` Function, Which Updates The Velocities Of Colliding Particles To Simulate A Physically Accurate Separation.

* **Gravitational Simulation & Continuous Forces:** A Mild Gravitational Pull Is Introduced, Continuously Nudging Particles Downward (Negative Y-Direction), And Frictional Damping Has Also Been Applied To Gradually Reduce Velocity Over Time. These Forces, Combined With Repulsive Collisions, Produce More Visually Rich And Physically Intuitive Behavior. Gravity Is Simulated Through A Constant Update To The `particles.velocities` Array In The `updateParticlesThreaded(...)` Function, While Frictional Damping Is Applied By Scaling The Velocities To Simulate Energy Loss.

* **Performance & Scalability Enhancements:** Patch 2 Supports Thousands Of Particles (E.G., 5000) While Maintaining Stable Framerates. This Scalability Is Achieved Via Efficient Spatial Partitioning Through The `collisionMatrix` Object And Multi-Threading. The Core Principles From Patch 1—Using Spatial Hashing To Reduce Brute-Force Collision Complexity—Still Hold, But We Have Tuned Parameters (Like `BIN_SIZE` And `NUM_THREADS`) And Refined Our Data Management To Further Increase Efficiency. Threads Are Launched Using The Loop In `ParticleWindow`'s Constructor, And Each Thread Calls The `updateParticlesThreaded(...)` Function To Handle A Partition Of Particles.

* **Mouse Interactions With Refined Force Applications:** Similar To Patch 1, Clicking Applies Forces To Particles. However, The Force Model Is Updated: Left-Clicking Repels Particles And Right-Clicking Pulls Them Closer. In Patch 2, These Forces Are More Continuous And Subtle, Allowing For Smoother And More Granular Control Over Particle Dispersal Patterns. Keeping The Mouse Held Down Continuously Applies A Gentle, Ongoing Force, Enhancing User Interactivity And Making The System Feel More Dynamic And "Fluid." The Force Application Logic Is Handled By The `applyRepulsiveForce(...)` And `applyRepulsiveForceContinual(...)` Functions, Which Adjust The `particles.velocities` Array To Move Particles Based On The Mouse Position.

* **Overhauled Particle Initialization:** Particle Generation Now Uses Different Distributions For Radii, Positions, And Colors, Producing Visually Distinct Scenarios And Providing A More Diverse Particle Set. The Code Also Includes Subtle Runtime Dampening And Restitution Factors To Ensure Stable Long-Term Behavior. This Initialization Is Handled In The `initParticles(...)` Function, Where Each Particle's Attributes (Like `particles.radii`, `particles.colors`, `particles.velocities`, Etc.) Are Randomized Using Distributions To Achieve Diversity And Dynamic Interactions.

* **Threading & Synchronization:** Building On Patch 1’s Multithreading, Patch 2 Increases The Number Of Worker Threads (E.G., From 2 To 3) And Coordinates Them More Carefully. With Improved Synchronization Primitives, We Can Achieve Smooth Real-Time Performance Despite A Higher Particle Count And More Complex Collision Logic. The `ThreadSynchronizer` Class Continues To Manage Thread Barriers, Ensuring Consistent Frame-By-Frame Updates Without Data Races Or Visual Artifacts. The `updateBarrier` And `renderBarrier` Synchronize Threads To Maintain Proper Frame Timing, While Each Thread Calls The `updateParticlesThreaded(...)` Function To Update Its Assigned Partition.

_**Runtime Complexity & Hashing:**_ As With Patch 1, The Collision Detection Pipeline Uses Spatial Hashing To Keep Complexity Near O(N). By Assigning Each Particle To A Grid Cell Using The `collisionMatrix.addParticle(...)` Function And Only Checking For Collisions Among Neighbors Via `collisionMatrix.getPotentialColliders(...)`, We Avoid O(N²) Complexity. Patch 2 Further Refines These Operations, Balancing Cell Sizes, Threading Divisions, And Update Phases. This Ensures That Even As The Particle Count Scales, The Simulation Remains Performant.

**Technical Highlights:**

* **Spatial Partitioning With Fixed-Grid Hashing:** Each Frame, Particles Are Reinserted Into A Hash-Based Grid Structure Using The `collisionMatrix.addParticle(...)` Function. Collisions Are Detected By Querying Neighboring Cells Using The `collisionMatrix.getPotentialColliders(...)` Function, Ensuring Efficient O(1)-Like Lookups And Minimal Overhead When Scaling Up To Thousands Of Particles.

* **Elastic Collisions & Restitution:** Collision Resolution Now Uses Physical Principles Of Restitution And Force Multipliers, Ensuring That Particles Bounce Off Each Other Rather Than Sticking. This Process Is Implemented In The `resolveCollision(...)` Function, Which Computes The Impulse Response Between Two Colliding Particles And Updates Their Velocities Using Their Masses And Current Velocities.

* **Gravity & Frictional Dampening:** A Constant Downward Force Simulates Gravity, Causing Particles To Settle Over Time, While Friction-Like Velocity Reductions Prevent Runaway Accelerations And Maintain Long-Term Stability. Gravity Is Updated In The `updateParticlesThreaded(...)` Function By Applying A Constant Velocity Decrease, While Friction Is Simulated By Multiplying The Velocities By A Decay Factor.

* **Refined Rendering And Vertex Buffer Updates:** Just Like Patch 1, We Use A Struct-Of-Arrays (SoA) Design For Particle Data, Coupled With A Single Vertex Array And Parallel Rendering. Patch 2 Scales Up The Rendering Pipeline To Handle A Larger Particle Set Efficiently. Drawing Thousands Of Circles At 60-90 FPS Remains Smooth Due To Careful Memory Handling And Incremental Updates To OpenGL Buffers Using The `renderParticles(...)` Function, Which Uploads The `vertices` Array To The GPU Using OpenGL's `glBufferSubData(...)`.


<img src="https://github.com/user-attachments/assets/0c481edf-693f-4b4b-bca2-f6017a3e15d4" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/0c481edf-693f-4b4b-bca2-f6017a3e15d4" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/0c481edf-693f-4b4b-bca2-f6017a3e15d4" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/0c481edf-693f-4b4b-bca2-f6017a3e15d4" alt="Cornstarch <3" width="55" height="49"> 

----------------------------------------------

<img src="https://github.com/user-attachments/assets/69234949-e52b-4056-9b5b-382fa1f28745" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/69234949-e52b-4056-9b5b-382fa1f28745" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/69234949-e52b-4056-9b5b-382fa1f28745" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/69234949-e52b-4056-9b5b-382fa1f28745" alt="Cornstarch <3" width="55" height="49"> 


<h3>Features:</h3>


![Untitledvideo5-ezgif com-optimize](https://github.com/user-attachments/assets/8bf7c276-1cbb-4507-a59f-dfb677b61965)

![Untitledvideo-ezgif com-optimize (6)](https://github.com/user-attachments/assets/547054bc-975b-4d72-9e88-8fdf49e26953)

![image](https://github.com/user-attachments/assets/d2127f59-015e-4e44-9780-0f92d3efffaf)

![image](https://github.com/user-attachments/assets/98386363-f9f4-4c4b-a816-90b62bd22ede)

![image](https://github.com/user-attachments/assets/26778f97-a2fa-4d04-9c34-b98930104987)


<img src="https://github.com/user-attachments/assets/0363619c-11fb-472f-b338-39b361304dd5" alt="Cornstarch <3" width="55" height="69"> <img src="https://github.com/user-attachments/assets/0363619c-11fb-472f-b338-39b361304dd5" alt="Cornstarch <3" width="55" height="69"> <img src="https://github.com/user-attachments/assets/0363619c-11fb-472f-b338-39b361304dd5" alt="Cornstarch <3" width="55" height="69"> <img src="https://github.com/user-attachments/assets/0363619c-11fb-472f-b338-39b361304dd5" alt="Cornstarch <3" width="55" height="69">

