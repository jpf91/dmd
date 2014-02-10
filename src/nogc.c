// Compiler implementation of the D programming language
// Copyright (c) 1999-2013 by Digital Mars
// All Rights Reserved
// written by Walter Bright
// http://www.digitalmars.com
// License for redistribution is by either the Artistic License
// in artistic.txt, or the GNU General Public License in gnu.txt.
// See the included readme.txt for details.

#include "mars.h"
#include "init.h"
#include "visitor.h"
#include "expression.h"
#include "statement.h"
#include "declaration.h"
#include "id.h"


void walkPreorder(Statement *s, StoppableVisitor *v);

/**************************************
 * This recursively visits all expressions used in a statement
 * and checks if they cause GC allocations. Calls func->setGCUse
 * in case an allocation is found. 
 */
class GCUseVisitor : public StoppableVisitor
{
public:
    FuncDeclaration *func;

    GCUseVisitor(FuncDeclaration *fdecl)
    {
        func = fdecl;
    }

    void doCond(Expression *e)
    {
        if (e && !e->visiting)
            e->accept(this);
    }
    void doCond(Initializer *i)
    {
        if (i)
            i->accept(this);
    }

    void visit(Statement *s)
    {
    }
    void visit(ExpStatement *s)
    {
        doCond(s->exp);
    }
    void visit(CompileStatement *s)
    {
    }
    void visit(WhileStatement *s)
    {
        doCond(s->condition);
    }
    void visit(DoStatement *s)
    {
        doCond(s->condition);
    }
    void visit(ForStatement *s)
    {
        doCond(s->condition);
        doCond(s->increment);
    }
    void visit(ForeachStatement *s)
    {
        doCond(s->aggr);
    }
    void visit(ForeachRangeStatement *s)
    {
        doCond(s->lwr);
        doCond(s->upr);
    }
    void visit(IfStatement *s)
    {
        doCond(s->condition);
    }
    void visit(PragmaStatement *s)
    {
        for (size_t i = 0; i < s->args->dim; i++)
            doCond((*s->args)[i]);
    }
    void visit(SwitchStatement *s)
    {
        doCond(s->condition);
    }
    void visit(CaseStatement *s)
    {
        doCond(s->exp);
    }
    void visit(CaseRangeStatement *s)
    {
        doCond(s->first);
        doCond(s->last);
    }
    void visit(ReturnStatement *s)
    {
        doCond(s->exp);
    }
    void visit(SynchronizedStatement *s)
    {
        doCond(s->exp);
    }
    void visit(WithStatement *s)
    {
        doCond(s->exp);
    }
    void visit(ThrowStatement *s)
    {
        doCond(s->exp);
    }

    void visit(Initializer *init)
    {
    }
    void visit(StructInitializer *init)
    {
        for (size_t i = 0; i < init->value.dim; i++)
            doCond(init->value[i]);
    }
    void visit(ArrayInitializer *init)
    {
        for (size_t i = 0; i < init->value.dim; i++)
            doCond(init->value[i]);
    }
    void visit(ExpInitializer *init)
    {
        doCond(init->exp);
    }

    void visit(Expression *e)
    {
    }
    void visit(TupleExp *e)
    {
        e->visiting = true;
        doCond(e->e0);
        for (size_t i = 0; i < e->exps->dim; i++)
            doCond((*e->exps)[i]);
        e->visiting = false;
    }
    void visit(StructLiteralExp *e)
    {
        e->visiting = true;
        for (size_t i = 0; i < e->elements->dim; i++)
            doCond((*e->elements)[i]);
        e->visiting = false;
    }
    void visit(UnaExp *e)
    {
        e->visiting = true;
        doCond(e->e1);
        e->visiting = false;
    }
    void visit(BinExp *e)
    {
        e->visiting = true;
        doCond(e->e1);
        doCond(e->e2);
        e->visiting = false;
    }
    void visit(SliceExp *e)
    {
        e->visiting = true;
        doCond(e->e1);
        doCond(e->upr);
        doCond(e->lwr);
        e->visiting = false;
    }
    void visit(DeclarationExp *e)
    {
        VarDeclaration *var = e->declaration->isVarDeclaration();
        if (var)
        {
            //Handle int[4] arr = [1,2,3,4];
            if (var && var->type && var->type->ty == Tsarray && var->init && var->init->isExpInitializer() &&
                var->init->isExpInitializer()->exp->op == TOKconstruct)
            {
                ConstructExp *cons = (ConstructExp*)var->init->isExpInitializer()->exp;
                if(cons->e2 && cons->e2->op == TOKarrayliteral)
                {
                    return;
                }
            }
            e->visiting = true;
            doCond(var->init);
            e->visiting = false;
        }
        
    }
    void visit(ArrayExp *e)
    {
        e->visiting = true;
        for (size_t i = 0; i < e->arguments->dim; i++)
            doCond((*e->arguments)[i]);
        e->visiting = false;
    }
    void visit(CallExp *e)
    {
        visit((UnaExp *)e);

        e->visiting = true;
        for (size_t i = 0; i < e->arguments->dim; i++)
            doCond((*e->arguments)[i]);
        e->visiting = false;

        if (e->e1 && e->e1->op == TOKvar)
        {
            VarExp *ve = (VarExp*)e->e1;
            if (ve->var && ve->var->isFuncDeclaration() && ve->var->isFuncDeclaration()->ident)
            {
                Identifier *ident = ve->var->isFuncDeclaration()->ident;

                if (strcmp(ident->toChars(), "_adSort") == 0 ||
                    strcmp(ident->toChars(), "_adSortChar") == 0 ||
                    strcmp(ident->toChars(), "_adSortWchar") == 0)
                {
                    if (func->setGCUse(e->loc, "'sort' may cause gc allocation"))
                    {
                        e->error("Can not use 'sort' in @nogc code");
                    }
                }
                if (ident == Id::adDup)
                {
                    if (func->setGCUse(e->loc, "'dup' causes gc allocation"))
                    {
                        e->error("Can not use 'dup' in @nogc code");
                    }
                }
            }
        }
    }
    void visit(CatExp *e)
    {
        visit((BinExp *)e);
        if (func->setGCUse(e->loc, "Concatenation may cause gc allocation"))
            e->error("Can not use concatenation in @nogc code");
    }
    void visit(CatAssignExp *e)
    {
        visit((BinExp *)e);
        if (func->setGCUse(e->loc, "Concatenation may cause gc allocation"))
            e->error("Can not use concatenation in @nogc code");
    }
    void visit(AssignExp *e)
    {
        visit((BinExp *)e);
        if (e->e1->op == TOKarraylength
           && func->setGCUse(e->loc, "Setting 'length' may cause gc allocation"))
        {
            e->error("Can not set 'length' in @nogc code");
        }
    }
    void visit(DeleteExp *e)
    {
        if (func->setGCUse(e->loc, "'delete' requires gc"))
            e->error("Can not use 'delete' in @nogc code");
    }
    void visit(NewExp *e)
    {
        if (e->arguments)
        {
            e->visiting = true;
            for (size_t i = 0; i < e->arguments->dim; i++)
                doCond((*e->arguments)[i]);
            e->visiting = false;
        }

        if (!e->allocator && !e->onstack && func->setGCUse(e->loc, "'new' causes gc allocation"))
            e->error("Can not use 'new' in @nogc code");
    }
    void visit(NewAnonClassExp *e)
    {
        if (e->arguments)
        {
            e->visiting = true;
            for (size_t i = 0; i < e->arguments->dim; i++)
                doCond((*e->arguments)[i]);
            e->visiting = false;
        }

        if (func->setGCUse(e->loc, "'new' causes gc allocation"))
            e->error("Can not use 'new' in @nogc code");
    }
    void visit(AssocArrayLiteralExp *e)
    {
        e->visiting = true;
        if (e->keys)
        {
            for (size_t i = 0; i < e->keys->dim; i++)
                doCond((*e->keys)[i]);
        }
        if (e->values)
        {
            for (size_t i = 0; i < e->values->dim; i++)
                doCond((*e->values)[i]);
        }
        e->visiting = false;

        if (e->keys->dim
            && func->setGCUse(e->loc, "Associative array literals cause gc allocation"))
        {
            e->error("Can not use associative array literals in @nogc code");
        }
    }
    void visit(ArrayLiteralExp *e)
    {
        if (e->elements)
        {
            e->visiting = true;
            for (size_t i = 0; i < e->elements->dim; i++)
                doCond((*e->elements)[i]);
            e->visiting = false;
        }

        if (e->elements && e->elements->dim
            && func->setGCUse(e->loc, "Array literals cause gc allocation"))
        {
                e->error("Can not use array literals in @nogc code");
        }
    }
    void visit(IndexExp* e)
    {
        visit((BinExp *)e);

        if (e->e1->type->ty == Taarray && func->setGCUse(e->loc, "Indexing an associative array may cause gc allocation"))
            e->error("Can not index an associative array in @nogc code");
    }
};

void checkGC(FuncDeclaration *func, Statement *stmt)
{
    if (global.params.nogc || global.params.vgc)
    {
        GCUseVisitor gcv(func);
        walkPreorder(stmt, &gcv);
    }
}
