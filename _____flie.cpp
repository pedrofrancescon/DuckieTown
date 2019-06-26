#include "flie.h"

fuzzy_control fc;

fuzzy_set fs_near,fs_far, fs_soft, fs_hard;

linguisticvariable Distance, BreakControl;

rule infrule[2];

int main()
{
    
    fs_near.setname("Near");
    fs_near.setrange(0,50);
    fs_near.setval(0,0,10,20);
    
    fs_far.setname("Far");
    fs_far.setrange(0,50);
    fs_far.setval(0,10,20,50);
    
    fs_soft.setname("Soft");
    fs_soft.setrange(0,100);
    fs_soft.setval(0,0,75);
    
    fs_hard.setname("Hard");
    fs_hard.setrange(0,100);
    fs_hard.setval(0,75,100);
    
    Distance.setname("Distance");
    Distance.includecategory(&fs_near);
    Distance.includecategory(&fs_far);
    
    BreakControl.setname("Break");
    BreakControl.includecategory(&fs_soft);
    BreakControl.includecategory(&fs_hard);
    
    fc.definevars(Distance, BreakControl);
    
    fc.set_defuzz(CENTROID);
    
    // If Distance IS Near THEN BreakControl IS Hard
    fc.insert_rule("Near","Hard");
    
    // If Distance IS Far THEN BreakControl IS Soft
    fc.insert_rule("Far","Soft");
    
    // Variables for Input and Output
    float dist = 50.0;
    float breakPercentage;
    
    breakPercentage = fc.make_inference(dist);
    
    printf("for Distance %3.2f meters the defuzyfied Output for BreakControl is %3.2f \n",dist, breakPercentage);
    
    return(0);
}
