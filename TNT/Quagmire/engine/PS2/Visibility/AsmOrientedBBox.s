#include <machine/regdef.h>

/***********************************************************
* AsmOrientedBBox.s - an oriented bounding box routine...
*
* Written by:       Darrin Stewart
*                   6/30/01
***********************************************************/

    .balign 8
    .set noreorder
    .section .text
    
    .global _asm_WorldOBBoxInViewBasic
    .global _asm_WorldOBBoxInViewAdvanced

    .set noat


/***********************************************************
* extern "C" s32 _asm_WorldOBBoxInViewBasic( vector3* Corner
*                                       vector3* Axes
*                                       vector3* PlaneN
*                                       vector3* PlaneD
*                                       xbool TrivialAccept
*                                       xbool AllowNearZIntersection );
*
*   Inputs:  $a0 - Corner
*            $a1 - Axes
*            $a2 - PlaneN
*            $a3 - PlaneD
*            $t0 - TrivialAccept
*            $t1 - AllowNearZIntersection
*
*   Outputs: $v0 - Result (0 = OUTSIDE_VIEW, 1 = INSIDE_VIEW, 2 = PARTIAL_VIEW, 3 = INDETERMINATE
*
*   Register Usage:
*            $t2         counter
*            $t3         NIntersections
*            $t4         pPlaneN
*            $t5         pPlaneD
*            f0         const ZERO
*            f1         ftemp
*            f2..f4     BestPt
*            f5..f7     WorstPt
*            f8         -*pPlaneD
*            f9..f11    *pPlaneN
*            f23..f25   Axes[0]
*            f26..f28   Axes[1]
*            f29..f31   Axes[2]
*
***********************************************************/

_asm_WorldOBBoxInViewBasic:
        /* push registers that need to be saved onto the stack */
        sub     $sp, $sp, 0x30        # stack -= 48 bytes
        swc1    $f31, 0x00($sp)
        swc1    $f30, 0x04($sp)
        swc1    $f29, 0x08($sp)
        swc1    $f28, 0x0c($sp)
        swc1    $f27, 0x10($sp)
        swc1    $f26, 0x14($sp)
        swc1    $f25, 0x18($sp)
        swc1    $f24, 0x1c($sp)
        swc1    $f23, 0x20($sp)

        /* initial setup...init vars and load axes */
        addi    $t3, $0, 0x0    # NIntersections = 0
        lwc1   $f23, 0x00($a1)  # load Axes[0].X
        addi    $t2, $0, 0x6    # count = 6
        lwc1   $f24, 0x04($a1)  # load axes[0].Y
        addi    $t4, $a2, 0x00  # pPlaneN = &PlaneN[0]
        lwc1   $f25, 0x08($a1)  # load axes[0].Z
        addi    $t5, $a3, 0x00  # pPlaneD = &PlaneD[0]
        lwc1   $f26, 0x0c($a1)  # load axes[1].X
        lwc1   $f27, 0x10($a1)  # load axes[1].Y
        lwc1   $f28, 0x14($a1)  # load axes[1].Z
        lwc1   $f29, 0x18($a1)  # load axes[2].X
        lwc1   $f30, 0x1c($a1)  # load axes[2].Y
        lwc1   $f31, 0x20($a1)  # load axes[2].Z
        mtc1    $0, $f0         # load 0.0f into const float register

int_loop:
        /* #### WARNING--PIPELINING GOING ON HERE, WILL MAKE THE CODE MESSY!!!! #### */
        /* loading the plane and setting the best/worst pts. up */
        /*
        if ( Axes[0].Dot( *pPlaneN ) > 0.0f )  BestPt += Axes[0];
        else                                   WorstPt += Axes[0];
        */
        lwc1    $f8,  0x0($t5)  # PlaneD = *pPlaneD
        lwc1    $f9,  0x0($t4)  # PlaneN = *pPlaneN
        lwc1    $f10, 0x4($t4)
        lwc1    $f11, 0x8($t4)
        lwc1    $f2,  0x0($a0)  # BestPoint.X = Corner.X
        lwc1    $f5,  0x0($a0)  # WorstPoint.X = Corner.X
        mula.s  $f23, $f9       # Axes[0].Dot( *pPlaneN )
        lwc1    $f3,  0x4($a0)  # BestPoint.Y = Corner.Y
        madda.s $f24, $f10
        lwc1    $f6,  0x4($a0)  # WorstPoint.Y = Corner.Y
        madd.s  $f1, $f25, $f11
        lwc1    $f4,  0x8($a0)  # BestPoint.Z = Corner.Z
        lwc1    $f7,  0x8($a0)  # WorstPoint.Z = Corner.Z
        c.olt.s $f0, $f1        # iff ( 0 < Dot )
        neg.s   $f8, $f8        # negate *pPlaneD
        bc1t    best_point0
        nop                     # (BDS)
        add.s   $f5, $f5, $f23  # WorstPt += Axes[0]
        add.s   $f6, $f6, $f24
        j       worst_point0
        add.s   $f7, $f7, $f25  # (BDS)
best_point0:
        add.s   $f2, $f2, $f23  # BestPt += Axes[0]
        add.s   $f3, $f3, $f24
        add.s   $f4, $f4, $f25

worst_point0:
        /*
        if ( Axes[1].Dot( *pPlaneN ) > 0.0f )  BestPt += Axes[1];
        else                                   WorstPt += Axes[1];
        */
        mula.s  $f26, $f9       # Axes[1].Dot( *pPlaneN )
        madda.s $f27, $f10
        madd.s  $f1, $f28, $f11
        c.olt.s $f0, $f1        # iff ( 0 < Dot )
        nop
        bc1t    best_point1
        nop                     # (BDS)
        add.s   $f5, $f5, $f26  # WorstPt += Axes[1]
        add.s   $f6, $f6, $f27
        j       worst_point1
        add.s   $f7, $f7, $f28
best_point1:
        add.s   $f2, $f2, $f26  # BestPt += Axes[1]
        add.s   $f3, $f3, $f27
        add.s   $f4, $f4, $f28

worst_point1:
        /*
        if ( Axes[2].Dot( *pPlaneN ) > 0.0f )  BestPt += Axes[2];
        else                                   WorstPt += Axes[2];
        */
        mula.s  $f29, $f9       # Axes[2].Dot( *pPlaneN )
        madda.s $f30, $f10
        madd.s  $f1, $f31, $f11
        c.olt.s $f0, $f1        # iff ( 0 < Dot )
        nop
        bc1t    best_point2
        nop                     # (BDS)
        add.s   $f5, $f5, $f29  # WorstPt += Axes[2]
        add.s   $f6, $f6, $f30
        j       worst_point2
        add.s   $f7, $f7, $f31
best_point2:
        add.s   $f2, $f2, $f29  # BestPt += Axes[2]
        add.s   $f3, $f3, $f30
        add.s   $f4, $f4, $f31

worst_point2:
        /*
        if ( *pPlaneN.Dot( BestPt ) < -*PlaneD ) return OUTSIDE_VIEW;
        */
        mula.s  $f2, $f9            # pPlaneN->Dot( BestPt )
        madda.s $f3, $f10
        madd.s  $f1, $f4, $f11
        c.olt.s $f1, $f8            # iff ( Dot < -*pPlaneD )
        nop
        bc1t    finished_basic_test
        addi    $v0, $0, 0          # (BDS) Res = OUTSIDE_VIEW

        /*
        if ( *pPlaneN.Dot( WorstPt ) >= -*pPlaneD ) continue;
        */
        mula.s  $f5, $f9            # pPlaneN->Dot( WorstPt )
        madda.s $f6, $f10
        madd.s  $f1, $f7, $f11
        c.ole.s $f8, $f1            # iff ( Dot < -*pPlaneD )
        nop
        bc1t    next_int_loop
        nop

        /*
        else 
        {
            if ( !AllowNearZIntersection && (pPlane == &PlaneN[0]) )  return OUTSIDE_VIEW;
            NIntersections++;
        }
        */
        bne     $t4, $a2, z_allowed_or_i    # iff (pPlane != &PlaneN[0])
        nop
        bne     $t1, $0, z_allowed_or_i     # iff (AllowNearZIntersection)
        nop
        j       finished_basic_test         # return OUTSIDE_VIEW
        addi    $v0, $0, 0                  # (BDS)

z_allowed_or_i:
        addi    $t3, $t3, 1     # NIntersections++

next_int_loop:
        addi    $t2, $t2, -1     # counter--
        addi    $t4, $t4, 0x0c  # pPlaneN++
        addi    $t5, $t5, 0x04  # pPlaneD++
        bgtz    $t2, int_loop
        nop                     # (BDS)
        /*
        if ( NIntersections == 0 )  return INSIDE_VIEW;
        */
        beq     $t3, $0, finished_basic_test
        addi    $v0, $0, 1           # (BDS)
        /*
        if ( TrivialAcceptOnly )    return PARTIAL_VIEW;
        */
        bne     $t0, $0, finished_basic_test
        addi    $v0, $0, 2           # (BDS)
        /*
        if ( NIntersections == 1 )  return PARTIAL_VIEW;
        */
        addi    $t2, $t3, -1
        nop
        beq     $t2, $0, finished_basic_test
        addi    $v0, $0, 2           # (BDS)
        /*
        if ( NIntersections == 6 )  return PARTIAL_VIEW
        */
        addi    $t2, $t3, -6
        beq     $t2, $0, finished_basic_test
        addi    $v0, $0, 2

        /****************************************
        PART TWO - THE RESULT WAS INDETERMINATE
                   SO WE WILL NOW DO POLY CLIP
                   TESTS...UGHH....
        *****************************************/
        addi    $v0, $0, 3

finished_basic_test:
        /* pop registers and exit */
        lwc1    $f23, 0x20($sp)
        lwc1    $f24, 0x1c($sp)
        lwc1    $f25, 0x18($sp)
        lwc1    $f26, 0x14($sp)
        lwc1    $f27, 0x10($sp)
        lwc1    $f28, 0x0c($sp)
        lwc1    $f29, 0x08($sp)
        lwc1    $f30, 0x04($sp)
        lwc1    $f31, 0x00($sp)
        addiu   $29, $29, 0x0030        # stack += 48 bytes
        jr      $ra
        nop


/***********************************************************
* extern "C" s32 _asm_WorldOBBoxInViewAdvanced( vector3* Corner
*                                          vector3* Axes
*                                          vector3* PlaneN
*                                          vector3* PlaneD );
*
*   Inputs:  $a0 - Corner
*            $a1 - Axes
*            $a2 - PlaneN
*            $a3 - PlaneD
*
*   Outputs: $v0 - Result (0 = OUTSIDE_VIEW, 2 = PARTIAL_VIEW)
*
*   Register Usage:
*            $t0        Addr of source buffer
*            $t1        Addr of dest buffer
*            $t2        temp0
*            $t3        temp1
*            $t4        temp2
*            $t5        temp3
*            $t6        temp4
*            $t7        temp5
*            $t8        NVerts
*            $f0..$f2   Axes[0]
*            $f3..$f5   Axes[1]
*            $f6..$f8   Axes[2]
*            $f9..$f11  Corner
*            $f28       const (0.001f)
*            $f29..$f31 temp vector
*
*   Stack layout:
*            0x0000     Clip Buffer A       (144 bytes)
*            0x0090     Clip Buffer B       (144 bytes)
*            0x0120     BoxSides            (96 bytes)
*            0x0180     Pushed $f25..$f31   (32 bytes)
*            0x01a0     saved ra            (16 bytes)
*            0x01b0     sizeof(Stack)       (416 bytes)
*            
***********************************************************/

_asm_WorldOBBoxInViewAdvanced:
        /* push saved registers onto stack */
        sub     $sp, $sp, 0x01b0        # allocate stack space
        sq      $ra, 0x1a0($sp)
        swc1    $f31, 0x198($sp)
        swc1    $f30, 0x194($sp)
        swc1    $f29, 0x190($sp)
        swc1    $f28, 0x18c($sp)
        swc1    $f27, 0x188($sp)
        swc1    $f26, 0x184($sp)
        swc1    $f25, 0x180($sp)

        /* In preparation for filling in the box sides, load
           up the corner and axes */
        lwc1    $f9, 0x00($a0)          # load corner
        lwc1    $f10, 0x04($a0)
        lwc1    $f11, 0x08($a0)
        lwc1    $f0, 0x00($a1)          # load Axes[0]
        lwc1    $f1, 0x04($a1)
        lwc1    $f2, 0x08($a1)
        lwc1    $f3, 0x0c($a1)          # load Axes[1]
        lwc1    $f4, 0x10($a1)
        lwc1    $f5, 0x14($a1)
        lwc1    $f6, 0x18($a1)          # load Axes[2]
        lwc1    $f7, 0x1c($a1)
        lwc1    $f8, 0x20($a1)

        /* BoxSides[0] = Corner */
        swc1    $f9,  0x0120($sp)
        swc1    $f10, 0x0124($sp)
        swc1    $f11, 0x0128($sp)

        /* BoxSides[1] = Corner + Axes[2] */
        add.s   $f29, $f9,  $f6
        add.s   $f30, $f10, $f7
        add.s   $f31, $f11, $f8
        swc1    $f29, 0x012c($sp)
        swc1    $f30, 0x0130($sp)
        swc1    $f31, 0x0134($sp)

        /* BoxSides[2] = Corner + Axes[1] */
        add.s   $f29, $f9,  $f3
        add.s   $f30, $f10, $f4
        add.s   $f31, $f11, $f5
        swc1    $f29, 0x0138($sp)
        swc1    $f30, 0x013c($sp)
        swc1    $f31, 0x0140($sp)

        /* BoxSides[3] = Corner + Axes[1] + Axes[2] */
        add.s   $f29, $f9,  $f3
        add.s   $f30, $f10, $f4
        add.s   $f31, $f11, $f5
        add.s   $f29, $f29, $f6
        add.s   $f30, $f30, $f7
        add.s   $f31, $f31, $f8
        swc1    $f29, 0x0144($sp)
        swc1    $f30, 0x0148($sp)
        swc1    $f31, 0x014c($sp)

        /* BoxSides[4] = Corner + Axes[0] */
        add.s   $f29, $f9,  $f0
        add.s   $f30, $f10, $f1
        add.s   $f31, $f11, $f2
        swc1    $f29, 0x0150($sp)
        swc1    $f30, 0x0154($sp)
        swc1    $f31, 0x0158($sp)

        /* BoxSides[5] = Corner + Axes[0] + Axes[2] */
        add.s   $f29, $f9,  $f0
        add.s   $f30, $f10, $f1
        add.s   $f31, $f11, $f2
        add.s   $f29, $f29, $f6
        add.s   $f30, $f30, $f7
        add.s   $f31, $f31, $f8
        swc1    $f29, 0x015c($sp)
        swc1    $f30, 0x0160($sp)
        swc1    $f31, 0x0164($sp)

        /* BoxSides[6] = Corner + Axes[0] + Axes[1] */
        add.s   $f29, $f9,  $f0
        add.s   $f30, $f10, $f1
        add.s   $f31, $f11, $f2
        add.s   $f29, $f29, $f3
        add.s   $f30, $f30, $f4
        add.s   $f31, $f31, $f5
        swc1    $f29, 0x0168($sp)
        swc1    $f30, 0x016c($sp)
        swc1    $f31, 0x0170($sp)

        /* BoxSides[7] = Corner = Axes[0] + Axes[1] + Axes[2] */
        add.s   $f29, $f9,  $f0
        add.s   $f30, $f10, $f1
        add.s   $f31, $f11, $f2
        add.s   $f29, $f29, $f3
        add.s   $f30, $f30, $f4
        add.s   $f31, $f31, $f5
        add.s   $f29, $f29, $f6
        add.s   $f30, $f30, $f7
        add.s   $f31, $f31, $f8
        swc1    $f29, 0x0174($sp)
        swc1    $f30, 0x0178($sp)
        swc1    $f31, 0x017c($sp)

        /* fill source buffer with poly 3,1,5,7 and clip */
        lw      $t2, 0x0144($sp)    # load vert 3
        lw      $t3, 0x0148($sp)
        lw      $t4, 0x014c($sp)
        lw      $t5, 0x012c($sp)    # load vert 1
        lw      $t6, 0x0130($sp)
        lw      $t7, 0x0134($sp)
        sw      $t2, 0x0000($sp)    # store vert 3
        sw      $t3, 0x0004($sp)
        sw      $t4, 0x0008($sp)
        sw      $t5, 0x000c($sp)    # store vert 1
        sw      $t6, 0x0010($sp)
        sw      $t7, 0x0014($sp)
        lw      $t2, 0x015c($sp)    # load vert 5
        lw      $t3, 0x0160($sp)
        lw      $t4, 0x0164($sp)
        lw      $t5, 0x0174($sp)    # load vert 7
        lw      $t6, 0x0178($sp)
        lw      $t7, 0x017c($sp)
        sw      $t2, 0x0018($sp)    # store vert 5
        sw      $t3, 0x001c($sp)
        sw      $t4, 0x0020($sp)
        sw      $t5, 0x0024($sp)    # store vert 7
        sw      $t6, 0x0028($sp)
        sw      $t7, 0x002c($sp)
        addi    $t0, $sp, 0x0000    # set SrcPtr
        addi    $t1, $sp, 0x0090    # set DstPtr
        jal     poly_in_view
        addi    $t8, $0, 4          # NVerts = 4;
        /* if ( NVerts ) return PARTIAL_VIEW */
        bgtz    $t8, finished_obb_advanced
        addi    $v0, $0, 2

        /* fill source buffer with poly 7, 5, 4, 6 and clip */
        lw      $t2, 0x0174($sp)    # load vert 7
        lw      $t3, 0x0178($sp)
        lw      $t4, 0x017c($sp)
        lw      $t5, 0x015c($sp)    # load vert 5
        lw      $t6, 0x0160($sp)
        lw      $t7, 0x0164($sp)
        sw      $t2, 0x0000($sp)    # store vert 7
        sw      $t3, 0x0004($sp)
        sw      $t4, 0x0008($sp)
        sw      $t5, 0x000c($sp)    # store vert 5
        sw      $t6, 0x0010($sp)
        sw      $t7, 0x0014($sp)
        lw      $t2, 0x0150($sp)    # load vert 4
        lw      $t3, 0x0154($sp)
        lw      $t4, 0x0158($sp)
        lw      $t5, 0x0168($sp)    # load vert 6
        lw      $t6, 0x016c($sp)
        lw      $t7, 0x0170($sp)
        sw      $t2, 0x0018($sp)    # store vert 4
        sw      $t3, 0x001c($sp)
        sw      $t4, 0x0020($sp)
        sw      $t5, 0x0024($sp)    # store vert 5
        sw      $t6, 0x0028($sp)
        sw      $t7, 0x002c($sp)
        addi    $t0, $sp, 0x0000    # set SrcPtr
        addi    $t1, $sp, 0x0090    # set DstPtr
        jal     poly_in_view
        addi    $t8, $0, 4          # NVerts = 4
        /* if ( NVerts ) return PARTIAL_VIEW */
        bgtz    $t8, finished_obb_advanced
        addi    $v0, $0, 2

        /* fill source buffer with poly 6,4,0,2 and clip */
        lw      $t2, 0x0168($sp)    # load vert 6
        lw      $t3, 0x016c($sp)
        lw      $t4, 0x0170($sp)
        lw      $t5, 0x0150($sp)    # load vert 4
        lw      $t6, 0x0154($sp)
        lw      $t7, 0x0158($sp)
        sw      $t2, 0x0000($sp)    # store vert 6
        sw      $t3, 0x0004($sp)
        sw      $t4, 0x0008($sp)
        sw      $t5, 0x000c($sp)    # store vert 4
        sw      $t6, 0x0010($sp)
        sw      $t7, 0x0014($sp)
        lw      $t2, 0x0120($sp)    # load vert 0
        lw      $t3, 0x0124($sp)
        lw      $t4, 0x0128($sp)
        lw      $t5, 0x0138($sp)    # load vert 2
        lw      $t6, 0x013c($sp)
        lw      $t7, 0x0140($sp)
        sw      $t2, 0x0018($sp)    # store vert 0
        sw      $t3, 0x001c($sp)
        sw      $t4, 0x0020($sp)
        sw      $t5, 0x0024($sp)    # store vert 2
        sw      $t6, 0x0028($sp)
        sw      $t7, 0x002c($sp)
        addi    $t0, $sp, 0x0000    # set SrcPtr
        addi    $t1, $sp, 0x0090    # set DstPtr
        jal     poly_in_view
        addi    $t8, $0, 4          # NVerts = 4
        /* if ( NVerts ) return PARTIAL_VIEW */
        bgtz    $t8, finished_obb_advanced
        addi    $v0, $0, 2

        /* fill source buffer with poly 2,0,1,3 and clip */
        lw      $t2, 0x0138($sp)    # load vert 2
        lw      $t3, 0x013c($sp)
        lw      $t4, 0x0140($sp)
        lw      $t5, 0x0120($sp)    # load vert 0
        lw      $t6, 0x0124($sp)
        lw      $t7, 0x0128($sp)
        sw      $t2, 0x0000($sp)    # store vert 2
        sw      $t3, 0x0004($sp)
        sw      $t4, 0x0008($sp)
        sw      $t5, 0x000c($sp)    # store vert 0
        sw      $t6, 0x0010($sp)
        sw      $t7, 0x0014($sp)
        lw      $t2, 0x012c($sp)    # load vert 1
        lw      $t3, 0x0130($sp)
        lw      $t4, 0x0134($sp)
        lw      $t5, 0x0144($sp)    # load vert 3
        lw      $t6, 0x0148($sp)
        lw      $t7, 0x014c($sp)
        sw      $t2, 0x0018($sp)    # store vert 1
        sw      $t3, 0x001c($sp)
        sw      $t4, 0x0020($sp)
        sw      $t5, 0x0024($sp)    # store vert 3
        sw      $t6, 0x0028($sp)
        sw      $t7, 0x002c($sp)
        addi    $t0, $sp, 0x0000    # set SrcPtr
        addi    $t1, $sp, 0x0090    # set DstPtr
        jal     poly_in_view
        addi    $t8, $0, 4          # NVerts = 4
        /* if ( NVerts ) return PARTIAL_VIEW */
        bgtz    $t8, finished_obb_advanced
        addi    $v0, $0, 2

        /* fill source buffer with poly 2,3,7,6 and clip */
        lw      $t2, 0x0138($sp)    # load vert 2
        lw      $t3, 0x013c($sp)
        lw      $t4, 0x0140($sp)
        lw      $t5, 0x0144($sp)    # load vert 3
        lw      $t6, 0x0148($sp)
        lw      $t7, 0x014c($sp)
        sw      $t2, 0x0000($sp)    # store vert 2
        sw      $t3, 0x0004($sp)
        sw      $t4, 0x0008($sp)
        sw      $t5, 0x000c($sp)    # store vert 3
        sw      $t6, 0x0010($sp)
        sw      $t7, 0x0014($sp)
        lw      $t2, 0x0174($sp)    # load vert 7
        lw      $t3, 0x0178($sp)
        lw      $t4, 0x017c($sp)
        lw      $t5, 0x0168($sp)    # load vert 6
        lw      $t6, 0x016c($sp)
        lw      $t7, 0x0170($sp)
        sw      $t2, 0x0018($sp)    # store vert 7
        sw      $t3, 0x001c($sp)
        sw      $t4, 0x0020($sp)
        sw      $t5, 0x0024($sp)    # store vert 6
        sw      $t6, 0x0028($sp)
        sw      $t7, 0x002c($sp)
        addi    $t0, $sp, 0x0000    # set SrcPtr
        addi    $t1, $sp, 0x0090    # set DstPtr
        jal     poly_in_view
        addi    $t8, $0, 4          # NVerts = 4
        /* if ( NVerts ) return PARTIAL_VIEW */
        bgtz    $t8, finished_obb_advanced
        addi    $v0, $0, 2

        /* fill source buffer with poly 5,1,0,4 and clip */
        lw      $t2, 0x015c($sp)    # load vert 5
        lw      $t3, 0x0160($sp)
        lw      $t4, 0x0164($sp)
        lw      $t5, 0x012c($sp)    # load vert 1
        lw      $t6, 0x0130($sp)
        lw      $t7, 0x0134($sp)
        sw      $t2, 0x0000($sp)    # store vert 5
        sw      $t3, 0x0004($sp)
        sw      $t4, 0x0008($sp)
        sw      $t5, 0x000c($sp)    # store vert 1
        sw      $t6, 0x0010($sp)    
        sw      $t7, 0x0014($sp)
        lw      $t2, 0x0120($sp)    # load vert 0
        lw      $t3, 0x0124($sp)
        lw      $t4, 0x0128($sp)
        lw      $t5, 0x0150($sp)    # load vert 4
        lw      $t6, 0x0154($sp)
        lw      $t7, 0x0158($sp)
        sw      $t2, 0x0018($sp)    # store vert 0
        sw      $t3, 0x001c($sp)
        sw      $t4, 0x0020($sp)
        sw      $t5, 0x0024($sp)    # store vert 4
        sw      $t6, 0x0028($sp)
        sw      $t7, 0x002c($sp)
        addi    $t0, $sp, 0x0000    # set SrcPtr
        addi    $t1, $sp, 0x0090    # set DstPtr
        jal     poly_in_view
        addi    $t8, $0, 4          # NVerts = 4
        /* if ( NVerts ) return PARTIAL_VIEW */
        bgtz    $t8, finished_obb_advanced
        addi    $v0, $0, 2

        /* return OUTSIDE_VIEW */
        addi    $v0, $0, 0

finished_obb_advanced:
        /* pop saved registers from stack */
        lwc1    $f25, 0x180($sp)
        lwc1    $f26, 0x184($sp)
        lwc1    $f27, 0x188($sp)
        lwc1    $f28, 0x18c($sp)
        lwc1    $f29, 0x190($sp)
        lwc1    $f30, 0x194($sp)
        lwc1    $f31, 0x198($sp)
        lq      $ra, 0x1a0($sp)
        addiu   $29, $29, 0x01b0        # dealloc stack space
        jr      $ra
        nop

/***********************************************************
*
*   poly_in_view
*
*   Inputs:  $a0 - Corner (From above)
*            $a1 - Axes   (From above)
*            $a2 - PlaneN (From above)
*            $a3 - PlaneD (From above)
*
*   Outputs: $t8 - NClippedVerts
*
*   Register Usage:
*            $t0        BufferA
*            $t1        BufferB
*            $t2        pSrc (for filling)
*            $t3        pDst (for filling)
*            $t4        
*            $t5
*            $t6
*            $t7
*            $t8        NVerts
*            $f0        ClipPlaneNX
*            $f1        ClipPlaneNY
*            $f2        ClipPlaneNZ
*            $f3        ClipPlaneD
*
*   Stack layout:
*            0x0000     ra (16 bytes)
*            0x0010     sizeof(Stack)
*            
***********************************************************/

poly_in_view:
        /* save ra on stack */
        sub     $sp, $sp, 0x0010        # allocate stack space
        sq      $ra, 0x0000($sp)        # store ra

        /* clip to near plane */
        addi    $t2, $t0, 0             # pSrc = BufferA
        addi    $t3, $t1, 0             # pDst = BufferB
        lwc1    $f0, 0x0000($a2)        # PlaneN = pPlaneN[0]
        lwc1    $f1, 0x0004($a2)
        lwc1    $f2, 0x0008($a2)
        jal     clip_to_plane
        lwc1    $f3, 0x0000($a3)        # PlaneD = pPlaneD[0]
        beq     $t8, $0, finished_poly_in_view
        nop

        /* clip to bottom plane */
        addi    $t2, $t1, 0             # pSrc = BufferB
        addi    $t3, $t0, 0             # pDst = BufferA
        lwc1    $f0, 0x0030($a2)        # PlaneN = pPlaneN[4]
        lwc1    $f1, 0x0034($a2)
        lwc1    $f2, 0x0038($a2)
        jal     clip_to_plane
        lwc1    $f3, 0x0010($a3)        # PlaneD = pPlaneD[4]
        beq     $t8, $0, finished_poly_in_view
        nop

        /* clip to left plane */
        addi    $t2, $t0, 0             # pSrc = BufferA
        addi    $t3, $t1, 0             # pDst = BufferB
        lwc1    $f0, 0x000c($a2)        # PlaneN = pPlaneN[1]
        lwc1    $f1, 0x0010($a2)
        lwc1    $f2, 0x0014($a2)
        jal     clip_to_plane
        lwc1    $f3, 0x0004($a3)        # PlaneD = pPlaneD[1]
        beq     $t8, $0, finished_poly_in_view
        nop
        
        /* clip to right plane */
        addi    $t2, $t1, 0             # pSrc = BufferB
        addi    $t3, $t0, 0             # pDst = BufferA
        lwc1    $f0, 0x0018($a2)        # PlaneN = pPlaneN[2]
        lwc1    $f1, 0x001c($a2)
        lwc1    $f2, 0x0020($a2)
        jal     clip_to_plane
        lwc1    $f3, 0x0008($a3)        # PlaneD = pPlaneD[2]
        beq     $t8, $0, finished_poly_in_view
        nop

        /* clip to top plane */
        addi    $t2, $t0, 0             # pSrc = BufferA
        addi    $t3, $t1, 0             # pDst = BufferB
        lwc1    $f0, 0x0024($a2)        # PlaneN = pPlaneN[3]
        lwc1    $f1, 0x0028($a2)
        lwc1    $f2, 0x002c($a2)
        jal     clip_to_plane
        lwc1    $f3, 0x000c($a3)        # PlaneD = pPlaneD[3]
        beq     $t8, $0, finished_poly_in_view
        nop

        /* clip to far plane */
        addi    $t2, $t1, 0             # pSrc = BufferB
        addi    $t3, $t0, 0             # pDst = BufferA
        lwc1    $f0, 0x003c($a2)        # PlaneN = pPlaneN[5]
        lwc1    $f1, 0x0040($a2)
        lwc1    $f2, 0x0044($a2)
        jal     clip_to_plane
        lwc1    $f3, 0x0014($a3)        # PlaneD = pPlaneD[5]

finished_poly_in_view:
        /* pop ra from stack */
        lq      $ra, 0x0000($sp)        # load
        addiu   $sp, $sp, 0x0010        # dealloc stack space
        jr      $ra
        nop

/***********************************************************
*
*   poly_in_view
*
*   Inputs:  $a0 - Corner (From above)
*            $a1 - Axes   (From above)
*            $a2 - PlaneN (From above)
*            $a3 - PlaneD (From above)
*            $t2 - src buffer ptr
*            $t3 - dst buffer ptr
*            $t8 - NSrcVerts (From above)
*            $f0 - ClipPlaneNX (From above)
*            $f1 - ClipPlaneNY (From above)
*            $f2 - ClipPlaneNZ (From above)
*            $f3 - ClipPlaneD (From above)
*
*   Outputs: $t8 - NClippedVerts
*
*   Register Usage:
*            $t0        BufferA (STATIC - DON'T TOUCH)
*            $t1        BufferB (STATIC - DON'T TOUCH)
*            $t2        pSrc (for filling)
*            $t3        pDst (for filling)
*            $t4        bool CurrIn
*            $t5        bool NextIn
*            $t6        temp
*            $t7        NDstVerts
*            $t8        NSrcVerts
*            $f0        ClipPlaneNX
*            $f1        ClipPlaneNY
*            $f2        ClipPlaneNZ
*            $f3        ClipPlaneD
*            $f4..$f6   CurrPoint
*            $f7..$f9   NextPoint
*            $f10       CurrDot
*            $f11       NextDot
*            $f28       const (0.001f)
*            $f30       temp (s)
*            $f31       temp (t)
*
*   Stack layout:
*            0x0000     ra (16 bytes)
*            0x0010     sizeof(Stack)
*            
***********************************************************/

clip_to_plane:

        /* load const 0.001f into $f28 */
        lui     $t4, 0x3a83
        ori     $t4, 0x126f
        mtc1    $t4, $f28

        /* Prime loop by setting curr pt to last point in list */
        addi    $t4, $t8, 0         # temp0 equals NSrcVerts
        addi    $t5, $t8, 0         # temp1 equals NSrcVerts
        sll     $t4, $t4, 3         # temp0 *= 8
        sll     $t5, $t5, 2         # temp1 *= 4
        add     $t6, $t4, $t5       # temp = NSrcVerts*12
        neg.s   $f3, $f3            # PlaneD = -PlaneD
        addi    $t6, $t6, -12       # temp -= 12
        add     $t6, $t6, $t2       # temp += pSrc
        lwc1    $f4, 0x00($t6)      # curr pt = pSrc[NVerts-1]
        lwc1    $f5, 0x04($t6)
        lwc1    $f6, 0x08($t6)
        addi    $t7, $0, 0          # NDstVerts = 0
        mula.s  $f4, $f0            # CurrPt.Dot(PlaneN)
        madda.s $f5, $f1
        madd.s  $f10, $f6, $f2
        c.ole.s $f3, $f10           # iff ( Dot >= -PlaneD )
        nop
        bc1t    start_clip_loop
        addi    $t4, $0, 1
        addi    $t4, $0, 0
start_clip_loop:
        /* get NextPt and bool NextIn */
        lwc1    $f7, 0x00($t2)      # NextPt = *pSrc++
        lwc1    $f8, 0x04($t2)
        lwc1    $f9, 0x08($t2)
        addi    $t2, $t2, 12
        mula.s  $f7, $f0            # NextPt.Dot(PlaneN)
        madda.s $f8, $f1
        madd.s  $f11, $f9, $f2
        c.ole.s $f3, $f11           # iff ( Dot >= -PlaneD )
        nop
        bc1t    next_is_in
        addi    $t5, $0, 1
        addi    $t5, $0, 0
next_is_in:
        /* if ( CurrIsIn ) *pDst++ = CurrPt */
        beq     $t4, $0, curr_is_out
        swc1    $f4, 0x00($t3)      # (BDS) *pDst = CurrPt
        swc1    $f5, 0x04($t3)
        swc1    $f6, 0x08($t3)
        addi    $t3, $t3, 12        # pDst++
        addi    $t7, $t7, 1         # NDstVerts++
curr_is_out:
        /* if ( CurrIsIn != NextIsIn ) *pDst++ = IntLine(CurrPt,NextPt) */
        beq     $t4, $t5, curr_in_equals_next_in
        sub.s   $f30, $f10, $f3     # (BDS) s = CurrDot - (-PlaneD)
        sub.s   $f31, $f11, $f3     # t = NextDot - (-PlaneD)
        sub.s   $f31, $f30, $f31    # t = s - t
        
        /* if line segment is parallel with plane, clip at midway point,
           otherwise figure out a parametric value to clip at. */
        c.ole.s $f28, $f31          # iff ( 0.001f <= t ) goto non_parallel
        nop
        bc1t    non_parallel
        nop
        neg.s   $f28, $f28
        c.ole.s $f31, $f28          # iff ( -0.001f >= t ) goto non_parallel
        nop
        bc1t    non_parallel
        neg.s   $f28, $f28

        /* t = 0.5f */
        lui     $t6, 0x3f00
        nop
        nop
        j       parallel
        mtc1    $t6, $f31

non_parallel:
        /* t = s / t */
        div.s   $f31, $f30, $f31

parallel:
        /* do the interpolation */
        sub.s   $f25, $f7, $f4      # V = Next-Curr
        sub.s   $f26, $f8, $f5
        sub.s   $f27, $f9, $f6
        mul.s   $f25, $f25, $f31    # V *= t
        mul.s   $f26, $f26, $f31
        mul.s   $f27, $f27, $f31
        add.s   $f25, $f25, $f4     # V += Curr
        add.s   $f26, $f26, $f5
        add.s   $f27, $f27, $f6
        swc1    $f25, 0x00($t3)      # *pDst = V
        swc1    $f26, 0x04($t3)
        swc1    $f27, 0x08($t3)
        addi    $t3, $t3, 12        # pDst++
        addi    $t7, $t7, 1         # NDstVerts++

curr_in_equals_next_in:

        /* do the next loop */
        addi    $t8, $t8, -1        # NSrc--
        mov.s   $f10, $f11          # CurrDot = NextDot
        mov.s   $f4, $f7            # CurrPt = NextPt
        mov.s   $f5, $f8
        mov.s   $f6, $f9
        bgtz    $t8, start_clip_loop
        addi    $t4, $t5, 0         # CurrIn = NextIn
    
        /* we are outta here! */
        jr      $ra
        addi    $t8, $t7, 0         # (BDS) NSrcVerts = NDestVerts

    .set reorder
