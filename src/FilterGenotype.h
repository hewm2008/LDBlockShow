#ifndef FilterGenotype_H_
#define FilterGenotype_H_

#include "./snp_hwe.h"

using namespace std ;

/*
int  print_usage_18()
{
	cout <<""
		"\n"
		"\tUsage: FilterGeno -InPut <in.genotype> -OutPut <out.genotype>\n"
		"\n"
		"\t\t-InPut     <str>   InPut file of genotype\n"
		"\t\t-OutPut    <str>   OutPut the filter file\n"
		"\n"
		"\t\t-Het      <float>  the max ratio of het allele[0.88]\n"
		"\t\t-Miss     <float>  the max ratio of miss allele[0.88]\n"
		"\t\t-MAF      <float>  filter the low minor allele frequency[0.0]\n"
		"\t\t-HWE      <float>  Exact test of Hardy-Weinberg Equilibrium for SNP Pvalue[1e-6]\n"
		"\t\t-Cut3base          Filter position with 3 allele[off]\n"
		"\t\t-help              show this help\n" 
		"\n";
	return 1;
}
*/

int parse_cmd_18(int argc, char **argv, Para_18 * para_18  )
{
//	if (argc <=2 ) {print_usage_18();return 0;}
	int err_flag = 0;

	for(int i = 1; i < argc || err_flag; i++)
	{
		if(argv[i][0] != '-')
		{
			cerr << "command option error! please check." << endl;
			return 0;
		}
		string flag=argv[i] ;
		flag=replace_all(flag,"-","");

		if (flag  == "InPut" )
		{
			if(i + 1 == argc) {LogLackArg(flag); return 0;}
			i++;
			para_18->input=argv[i];
		}
		else if (flag  == "Het" )
		{
			if(i + 1 == argc) {LogLackArg(flag); return 0;}
			i++;
			para_18->Het=atof(argv[i]);
		}
		else if (flag  == "MAF" )
		{
			if(i + 1 == argc) {LogLackArg(flag);return 0;}
			i++;
			para_18->MAF=atof(argv[i]);
		}
		else if (flag == "HWE")
		{
			if(i + 1== argc) {LogLackArg(flag);return 0;}
			i++;
			para_18->HWE=atof(argv[i]);
		}
		else if (flag  == "Miss" )
		{
			if(i + 1 == argc) {LogLackArg(flag);return 0;}
			i++;
			para_18->Miss=atof(argv[i]);
		}
		else if (flag  ==  "OutPut")
		{
			if(i + 1 == argc) {LogLackArg(flag);return 0;}
			i++;
			para_18->output=argv[i];
		}
		else if (flag  == "Cut3base")
		{
			para_18->Cut3base=1;
		}
	}
/*
		else if (flag  == "help") 		{			print_usage_18();return 0;		}
		else
		{
			cerr << "UnKnow argument -"<<flag<<endl;
			return 0;
		}
	}
	if  ( (para_18->input).empty() || (para_18->output).empty() )
	{
		cerr<< "lack argument for the must"<<endl ;
		return 0;
	}

	para_18->output=add_Asuffix(para_18->output);


*/
	return 1 ;
}


int Filter_genotype_main(int argc, char *argv[])
{
	Para_18 * para_18 = new Para_18;
	if( parse_cmd_18(argc, argv, para_18 )==0)
	{
		delete  para_18 ;
		return 0 ;
	}

	igzstream IN (para_18->input.c_str(),ifstream::in); // ifstream  + gz 
	ogzstream OUT (para_18->output.c_str());

	if(!IN.good())
	{
		cerr << "open IN File error: "<<para_18->input<<endl;
		return 1;
	}

	if(!OUT.good())
	{
		cerr << "open OUT File error: "<<para_18->output<<endl;
		return 1;
	}


	int BadMiss=0;
	int BadSite=0;
	int BadHete=0;
	int BadMAF=0;
	int BadHWE=0;


	map <string,string > SNP_back_Allele ;
	SNP_back_Allele["M"]="AC";SNP_back_Allele["K"]="GT";SNP_back_Allele["Y"]="CT";
	SNP_back_Allele["R"]="AG";SNP_back_Allele["W"]="AT";SNP_back_Allele["S"]="CG";
	SNP_back_Allele["C"]="CC";SNP_back_Allele["G"]="GG";SNP_back_Allele["T"]="TT";
	SNP_back_Allele["A"]="AA";
	SNP_back_Allele["-"]="NN"; SNP_back_Allele["N"]="NN";

	vector<string> inff ;
	int miss=0 ;
	char best_base='N';
	char sed_base='N';
	int Max=0;
	int SeD=0;
	int sample=1;
	int Base_Number=0;
	int Het_count=0;

	while(!IN.eof())
	{
		string  line ;
		getline(IN,line);
		if (line.length()<=0  || line[0] == '#')  { continue ; }
		inff.clear();
		split(line,inff," \t");
		map <char , int>  Count ;
		int AA=inff.size();
		Het_count=0;
		for (int ii=2 ; ii<AA ; ii++)
		{
			string A_tmp=SNP_back_Allele[inff[ii]];
			Count[A_tmp[0]]++;
			Count[A_tmp[1]]++;
			if ( A_tmp[0]  != A_tmp[1])
			{
				Het_count++;
			}
		}

		sample=(AA-2);
		miss=0 ;
		best_base='N';
		sed_base='N';
		Max=0;
		SeD=0;

		map <char,int>  :: iterator it=Count.begin();
		Base_Number=0;

		for ( ; it!=Count.end(); it++ )
		{
			if ( (it->first ) == 'N' )
			{
				miss=it->second;
				continue ;
			}
			else if ((it->second)  > Max )
			{
				SeD=Max;
				sed_base=best_base;
				Max=(it->second);
				best_base=it->first;
			}
			else if ( (it->second)  >= SeD )
			{
				SeD=(it->second);
				sed_base=it->first;
			}
			Base_Number++;
		}

		if ( ( (miss*1.0)/(sample*2.0) )>(para_18->Miss) )
		{
			BadMiss++;
			continue ;
		}
		else if ( (Het_count*1.0/sample)>(para_18->Het) ) 
		{
			BadHete++;
			continue;
		}
		else if ( ((SeD*1.0)/(sample*2.0))< (para_18->MAF))
		{
			BadMAF++;
			continue;
		}
		else if (Base_Number<2)
		{
			BadMAF++;
			continue ;
		}
		else if ( ((para_18->Cut3base)==1)   && ( Base_Number>2 ) )
		{
			BadSite++;
			continue ;
		}
		int aa_count= (Max-Het_count)/2;
		int bb_count= (SeD-Het_count)/2;
		double  p=SNPHWE(Het_count,aa_count,bb_count);
		if  (p< (para_18->HWE)) { BadHWE++ ;continue ;}
		OUT<<line<<"\n";
	}

	OUT.close();
	IN.close();


	delete para_18 ;

	if (BadSite!=0)
	{
		cout<<"#Warning skip non bi-allelic(Singleton/ThreeMulti allelic) site, and total skip allelic sites number is :"<<BadSite<<endl;
	}
	if (BadMiss!=0)
	{
		cout<<"#Warning skip high missing site, and total skip allelic sites number is :"<<BadMiss<<endl;
	}
	if (BadHete!=0)
	{
		cout<<"#Warning skip high Heterozygosity site, and total skip allelic sites number is :"<<BadHete<<endl;
	}
	if (BadMAF!=0)
	{
		cout<<"#Warning skip low Minor Allele Frequency site, and total skip allelic sites number is :"<<BadMAF<<endl;
	}
	if (BadHWE!=0)
	{
	    cout<<"#Warning skip low Pvalue of Hardy-Weinberg Equilibrium site , and total skip allelic sites number is :"<<BadHWE<<endl;
	}




	return 0;


}

#endif //FilterGenotype_H_
///////// swimming in the sky and flying in the sea ////////////
