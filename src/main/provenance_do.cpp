/*CXXR $Id$
 *CXXR
 *CXXR This file is part of CXXR, a project to refactor the R interpreter
 *CXXR into C++.  It may consist in whole or in part of program code and
 *CXXR documentation taken from the R project itself, incorporated into
 *CXXR CXXR (and possibly MODIFIED) under the terms of the GNU General Public
 *CXXR Licence.
 *CXXR 
 *CXXR CXXR is Copyright (C) 2008-9 Andrew R. Runnalls, subject to such other
 *CXXR copyrights and copyright restrictions as may be stated below.
 *CXXR 
 *CXXR CXXR is not part of the R project, and bugs and other issues should
 *CXXR not be reported via r-bugs or other R project channels; instead refer
 *CXXR to the CXXR website.
 *CXXR */

/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1999-2007   The R Development Core Team.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, a copy is available at
 *  http://www.r-project.org/Licenses/
 */

/** @file provenance_do.cpp
 *
 * @brief Provenance-related R functions
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "CXXR/RObject.h"
#include <Internal.h>

#include <fstream>
#include <locale>
#include <set>
#include <boost/archive/codecvt_null.hpp>
#include <boost/math/special_functions/nonfinite_num_facets.hpp>

#include "CXXR/Provenance.hpp"

// So that BOOST_CLASS_EXPORT is visible:
#include "CXXR/ListFrame.hpp"

// Try to get rid of this:
#include "Defn.h"

using namespace std;
using namespace CXXR;

SEXP attribute_hidden do_castestfun(SEXP call, SEXP op, SEXP args, SEXP rho)
{
	int n;
	if ((n=length(args))!=1)
		errorcall(call,_("%d arguments passed to 'castestfun' which requires 1"),n);

	if (TYPEOF(CAR(args))!=SYMSXP)
		errorcall(call,_("castestfun expects Symbol argument"));
    /*GCStackRoot<IntVector> v(GCNode::expose(new IntVector(3)));
    (*v)[0]=1;
    (*v)[1]=2;
    (*v)[2]=3;
    return v;*/
	/*GCStackRoot<LogicalVector> a(SEXP_downcast<LogicalVector*>(CAR(args)));
	GCStackRoot<LogicalVector> v(GCNode::expose(new LogicalVector(1)));
	if ((*a)[0]==true)
		(*v)[0]=true;
	else
		(*v)[0]=false;
	return v;*/
	Symbol* sym=SEXP_downcast<Symbol*>(CAR(args));
	Environment* env=static_cast<Environment*>(rho);
	// Let's try to get the binding for given symbol...
	Frame::Binding* binding = env->findBinding(sym).second;
	if (binding!=NULL)
		printf("Binding located :-)\n");
	GCStackRoot<IntVector> inv(GCNode::expose(new IntVector(3)));
	(*inv)[0]=1;
	(*inv)[1]=2;
	(*inv)[2]=3;
	GCStackRoot<IntVector> inv2(GCNode::expose(new IntVector(2)));
	(*inv2)[0]=4;
	(*inv2)[1]=5;

	GCStackRoot<StringVector> str(GCNode::expose(new StringVector(2)));
	(*str)[0]=const_cast<String*>(String::obtain("ivOne"));
	(*str)[1]=const_cast<String*>(String::obtain("ivTwo"));

	GCStackRoot<ListVector> rc(GCNode::expose(new ListVector(2)));
	(*rc)[0]=inv;
	(*rc)[1]=inv2;

	setAttrib(rc, R_NamesSymbol, str);
	return rc;
}

SEXP attribute_hidden do_hasProvenance (SEXP call, SEXP op, SEXP args, SEXP rho)
{
    int n;
    if ((n=length(args))!=1)
	errorcall(call,_("%d arguments passed to 'hasProvenance' which requires 1"),n);

    if (TYPEOF(CAR(args))!=SYMSXP)
	errorcall(call,_("hasProvenance expects Symbol argument"));

    GCStackRoot<LogicalVector> v(GCNode::expose(new LogicalVector(1)));
#ifdef PROVENANCE_TRACKING
    Symbol* sym=SEXP_downcast<Symbol*>(CAR(args));
    Environment* env=static_cast<Environment*>(rho);
    Frame::Binding* bdg = env->findBinding(sym).second;
    (*v)[0] = (bdg->provenance() != 0);
#else
    (*v)[0] = false;
#endif
    return v;
}

SEXP attribute_hidden do_provenance (SEXP call, SEXP op, SEXP args, SEXP rho)
{
#ifndef PROVENANCE_TRACKING
    Rf_error(_("provenance tracking not implemented in this build"));
    return 0;
#else
    const int nfields=5;
    int n;
    if ((n=length(args))!=1)
	errorcall(call,_("%d arguments passed to 'provenance' which requires 1"),n);

    if (TYPEOF(CAR(args))!=SYMSXP)
	errorcall(call,_("provenance expects Symbol argument"));
    Symbol* sym=SEXP_downcast<Symbol*>(CAR(args));
    Environment* env=static_cast<Environment*>(rho);
    Frame::Binding* bdg = env->findBinding(sym).second;
    if (!bdg)
	errorcall(call,_("invalid Symbol passed to 'provenance'"));
    Provenance* provenance=const_cast<Provenance*>(bdg->provenance());
    if (!provenance)
	errorcall(call,_("object does not have any provenance"));
    const Provenance::Set& children=provenance->children();

    GCStackRoot<ListVector> list(GCNode::expose(new ListVector(nfields)));
    GCStackRoot<StringVector> timestamp(GCNode::expose(new StringVector(1)));
    GCStackRoot<StringVector> names(GCNode::expose(new StringVector(nfields)));

    (*timestamp)[0]=const_cast<String*>(provenance->getTime());

    (*names)[0]=const_cast<String*>(String::obtain("command"));
    (*names)[1]=const_cast<String*>(String::obtain("symbol"));
    (*names)[2]=const_cast<String*>(String::obtain("timestamp"));
    (*names)[3]=const_cast<String*>(String::obtain("parents"));
    (*names)[4]=const_cast<String*>(String::obtain("children"));

    (*list)[0] = const_cast<RObject*>(provenance->command());
    (*list)[1] = const_cast<Symbol*>(provenance->symbol());
    (*list)[2]=timestamp;
    // Handle parents:
    {
	std::pair<CommandChronicle::ParentVector::const_iterator,
		  CommandChronicle::ParentVector::const_iterator>
	    pr = provenance->parents();
	size_t sz = pr.second - pr.first;
	StringVector* sv = CXXR_NEW(StringVector(sz));
	(*list)[3] = sv;
	unsigned int i = 0;
	for (CommandChronicle::ParentVector::const_iterator it = pr.first;
	     it != pr.second; ++it) {
	    const Provenance* p = *it;
	    (*sv)[i++] = const_cast<String*>(p->symbol()->name());
	}
    }
    if (!children.empty()) {
	StringVector* sv = CXXR_NEW(StringVector(children.size()));
	(*list)[4] = sv;
	unsigned int i = 0;
	for (Provenance::Set::const_iterator it = children.begin();
	     it != children.end(); ++it) {
	    const Provenance* p = *it;
	    (*sv)[i++] = const_cast<String*>(p->symbol()->name());
	}
    }

    setAttrib(list,R_NamesSymbol,names);

    return list;
#endif  // PROVENANCE_TRACKING
}

SEXP attribute_hidden do_provCommand (SEXP call, SEXP op, SEXP args, SEXP rho)
{
#ifndef PROVENANCE_TRACKING
    Rf_error(_("provenance tracking not implemented in this build"));
    return 0;
#else
    int n;
    if ((n=length(args))!=1)
	errorcall(call,_("%d arguments passed to 'provCommand' which requires 1"),n);

    if (TYPEOF(CAR(args))!=SYMSXP)
	errorcall(call,_("provCommand expects Symbol argument"));

    Symbol* sym=SEXP_downcast<Symbol*>(CAR(args));
    Environment* env=static_cast<Environment*>(rho);
    Frame::Binding* bdg = env->findBinding(sym).second;
    return const_cast<RObject*>(bdg->provenance()->command());
#endif  // PROVENANCE_TRACKING
}

SEXP attribute_hidden
do_provenance_graph(SEXP call, SEXP op, SEXP args, SEXP rho)
{
#ifndef PROVENANCE_TRACKING
    Rf_error(_("provenance tracking not implemented in this build"));
    return 0;
#else
    int nargs = length(args);
    if (nargs != 1)
	Rf_error(_("%d arguments passed to 'provenance.graph' which requires 1"),
		 nargs);
    SEXP arg1 = CAR(args);
    if (!arg1 || arg1->sexptype() != STRSXP)
	    Rf_error(_("invalid 'names' argument"));

    Environment* env = static_cast<Environment*>(rho);
    Provenance::Set provs;
    StringVector* sv = static_cast<StringVector*>(arg1);
    for (size_t i = 0; i < sv->size(); i++) {
	const char* name = (*sv)[i]->c_str();
	Symbol* sym = Symbol::obtain(name);
	Frame::Binding* bdg = env->findBinding(sym).second;
	if (!bdg)
	    Rf_error(_("symbol '%s' not found"), name);
	else {
	    Provenance* prov = const_cast<Provenance*>(bdg->provenance());
	    if (!prov)
		Rf_warning(_("'%s' does not have provenance information"),
			   name);
	    else provs.insert(prov);
	    }
	}

    Provenance::Set* ancestors = Provenance::ancestors(provs);

    GCStackRoot<ListVector> ans(CXXR_NEW(ListVector(7)));
    std::map<const Provenance*, unsigned int> ancestor_index;
    std::vector<std::pair<unsigned int, const RObject*> > xenogenous_bdgs;

    // Assemble information on graph nodes:
    {
	size_t n = ancestors->size();
	GCStackRoot<ListVector> symbols(CXXR_NEW(ListVector(n)));
	GCStackRoot<ListVector> commands(CXXR_NEW(ListVector(n)));
	GCStackRoot<RealVector> timestamps(CXXR_NEW(RealVector(n)));
	size_t i = 0;
	for (Provenance::Set::iterator it = ancestors->begin();
	     it != ancestors->end(); ++it) {
	    const Provenance* p = *it;
	    (*symbols)[i] = const_cast<Symbol*>(p->symbol());
	    (*commands)[i] = const_cast<RObject*>(p->command());
	    (*timestamps)[i] = p->timestamp();
	    ++i;
	    ancestor_index[p] = i;
	    if (p->isXenogenous())
		xenogenous_bdgs.push_back(std::make_pair(i, p->value()));
	}

	(*ans)[0] = symbols;
	(*ans)[1] = commands;
	(*ans)[2] = timestamps;
    }

    // Record information on xenogenous bindings:
    {
	size_t xn = xenogenous_bdgs.size();
	GCStackRoot<IntVector> xenogenous(CXXR_NEW(IntVector(xn)));
	GCStackRoot<ListVector> values(CXXR_NEW(ListVector(xn)));
	for (unsigned int i = 0; i < xn; ++i) {
	    std::pair<unsigned int, const RObject*>& pr = xenogenous_bdgs[i];
	    (*xenogenous)[i] = pr.first;
	    (*values)[i] = const_cast<RObject*>(pr.second);
	}
	(*ans)[3] = xenogenous;
	(*ans)[4] = values;
    }

    // Assemble information on graph edges:
    {
	typedef std::set<std::pair<unsigned int, unsigned int> > EdgeSet;
	EdgeSet edges;
	for (Provenance::Set::iterator it = ancestors->begin();
	     it != ancestors->end(); ++it) {
	    const Provenance* child = *it;
	    unsigned int child_idx = ancestor_index[child];
	    std::pair<CommandChronicle::ParentVector::const_iterator,
		      CommandChronicle::ParentVector::const_iterator>
		pr = child->parents();
	    for (CommandChronicle::ParentVector::const_iterator it = pr.first;
		 it != pr.second; ++it) {
		const Provenance* parent = *it;
		unsigned int parent_idx = ancestor_index[parent];
		edges.insert(std::make_pair(parent_idx, child_idx));
	    }
	}

	size_t en = edges.size();
	GCStackRoot<IntVector> parents(CXXR_NEW(IntVector(en)));
	GCStackRoot<IntVector> children(CXXR_NEW(IntVector(en)));
	unsigned int i = 0;
	for (EdgeSet::const_iterator it = edges.begin(); it != edges.end(); ++it) {
	    const std::pair<unsigned int, unsigned int>& edge = *it;
	    (*parents)[i] = edge.first;
	    (*children)[i] = edge.second;
	    ++i;
	}
		
	(*ans)[5] = parents;
	(*ans)[6] = children;
    }
    delete ancestors;
    return ans;
#endif  // PROVENANCE_TRACKING
}

namespace {
    // Import Bindings (and their provenance) from one Frame into another.
    void import(Frame* to, const Frame& from)
    {
	Frame::BindingRange range = from.bindingRange();
	for (Frame::BindingRange::const_iterator it = range.begin();
	     it != range.end(); ++it) {
	    const Frame::Binding& frombdg = *it;
	    Frame::Binding* tobdg = to->obtainBinding(frombdg.symbol());
#ifdef PROVENANCE_TRACKING
	    tobdg->setProvenance(const_cast<Provenance*>(frombdg.provenance()));
#endif
	    tobdg->setValue(frombdg.rawValue(), frombdg.origin(), TRUE);
	}
    }
}

SEXP attribute_hidden do_bserialize (SEXP call, SEXP op, SEXP args, SEXP rho)
{
    const int n = length(args);
    if (n > 0)
	Rf_errorcall(call,_("%d arguments passed to 'bserialize'"
			    " which requires 0"), n);

    ofstream ofs("bserialize.xml");
    
    // Refer to the Boost::Math documentation of 'Facets for
    // Floating-Point Infinities and NaNs' for the following runes:
    locale default_locale(locale::classic(),
			  new boost::archive::codecvt_null<char>);
    locale nfnum_locale(default_locale,
			new boost::math::nonfinite_num_put<char>);
    ofs.imbue(nfnum_locale);
    boost::archive::xml_oarchive oa(ofs, boost::archive::no_codecvt);

    GCStackRoot<Frame> frame(CXXR_NEW(StdFrame));
    GCStackRoot<Environment> env(CXXR_NEW(Environment(0, frame)));
    import(frame, *Environment::global()->frame());
    GCNPTR_SERIALIZE(oa, env);

    return 0;
}

SEXP attribute_hidden do_bdeserialize (SEXP call, SEXP op, SEXP args, SEXP rho)
{
    ifstream ifs("bserialize.xml");
    if (!ifs)
	Rf_error("file bserialize.xml not found");
    
    // Refer to the Boost::Math documentation of 'Facets for
    // Floating-Point Infinities and NaNs' for the following runes:
    locale default_locale(locale::classic(),
			  new boost::archive::codecvt_null<char>);
    locale nfnum_locale(default_locale,
			new boost::math::nonfinite_num_get<char>);
    ifs.imbue(nfnum_locale);
    boost::archive::xml_iarchive ia(ifs, boost::archive::no_codecvt);
    GCStackRoot<Environment> env;
    S11nScope scope;
    GCNPTR_SERIALIZE(ia, env);
    import(Environment::global()->frame(), *env->frame());
    return 0;
}
