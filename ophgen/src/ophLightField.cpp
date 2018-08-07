#include "ophLightField.h"

#include "sys.h"
#include "tinyxml2.h"

#define for_i(itr, oper) for(int i=0; i<itr; i++){ oper }

int ophLF::readLFConfig(const char* LF_config) {
	LOG("Reading....%s...", LF_config);

	auto start = CUR_TIME;

	/*XML parsing*/
	tinyxml2::XMLDocument xml_doc;
	tinyxml2::XMLNode *xml_node;

	if (checkExtension(LF_config, ".xml") == 0)
	{
		LOG("file's extension is not 'xml'\n");
		return false;
	}
	auto ret = xml_doc.LoadFile(LF_config);
	if (ret != tinyxml2::XML_SUCCESS)
	{
		LOG("Failed to load file \"%s\"\n", LF_config);
		return false;
	}

	xml_node = xml_doc.FirstChild();

	LF_directory = (xml_node->FirstChildElement("LightFieldImageDirectory"))->GetText();
	ext = (xml_node->FirstChildElement("LightFieldImageExtention"))->GetText();
#if REAL_IS_DOUBLE & true
	(xml_node->FirstChildElement("DistanceRS2Holo"))->QueryDoubleText(&distanceRS2Holo);
	(xml_node->FirstChildElement("SLMPixelPitchX"))->QueryDoubleText(&context_.pixel_pitch[_X]);
	(xml_node->FirstChildElement("SLMPixelPitchY"))->QueryDoubleText(&context_.pixel_pitch[_Y]);
	(xml_node->FirstChildElement("WavelengthofLaser"))->QueryDoubleText(&context_.lambda);
#else
	(xml_node->FirstChildElement("DistanceRS2Holo"))->QueryFloatText(&distanceRS2Holo);
	(xml_node->FirstChildElement("SLMPixelPitchX"))->QueryFloatText(&context_.pixel_pitch[_X]);
	(xml_node->FirstChildElement("SLMPixelPitchY"))->QueryFloatText(&context_.pixel_pitch[_Y]);
	(xml_node->FirstChildElement("WavelengthofLaser"))->QueryFloatText(&context_.lambda);
#endif
	(xml_node->FirstChildElement("NumberofImagesXofLF"))->QueryIntText(&num_image[_X]);
	(xml_node->FirstChildElement("NumberofImagesYofLF"))->QueryIntText(&num_image[_Y]);
	(xml_node->FirstChildElement("NumberofPixelXofLF"))->QueryIntText(&resolution_image[_X]);
	(xml_node->FirstChildElement("NumberofPixelYofLF"))->QueryIntText(&resolution_image[_Y]);
	(xml_node->FirstChildElement("EncodingMethod"))->QueryIntText(&ENCODE_METHOD);
	(xml_node->FirstChildElement("SingleSideBandPassBand"))->QueryIntText(&SSB_PASSBAND);

	context_.pixel_number[_X] = num_image[_X] * resolution_image[_X];
	context_.pixel_number[_Y] = num_image[_Y] * resolution_image[_Y];

	context_.k = (2 * M_PI) / context_.lambda;
	context_.ss[_X] = context_.pixel_number[_X] * context_.pixel_pitch[_X];
	context_.ss[_Y] = context_.pixel_number[_Y] * context_.pixel_pitch[_Y];

	auto end = CUR_TIME;

	auto during = ((std::chrono::duration<Real>)(end - start)).count();

	LOG("%.5lfsec...done\n", during);
	return true;
}

int ophLF::loadLF(const char* directory, const char* exten)
{
	LF_directory = directory;
	ext = exten;

	initializeLF();

	_finddata_t data;

	string sdir = std::string("./").append(LF_directory).append("/").append("*.").append(ext);
	intptr_t ff = _findfirst(sdir.c_str(), &data);
	if (ff != -1)
	{
		int num = 0;
		uchar* rgbOut;
		ivec2 sizeOut;
		int bytesperpixel;

		while (1)
		{
			//for_i(5, cout << "before: " << *(*LF + num) << endl;);
			//cin.get();

			string imgfullname = std::string(LF_directory).append("/").append(data.name);

			getImgSize(sizeOut[_X], sizeOut[_Y], bytesperpixel, imgfullname.c_str());
			rgbOut = new uchar[sizeOut[_X] * sizeOut[_Y] * 3];

			rgbOut = loadAsImg(imgfullname.c_str());

			if (rgbOut == 0) {
				cout << "LF load was failed." << endl;
				cin.get();
				return -1;
			}
				

			//for_i(sizeOut[_X] * sizeOut[_Y] * 3,
			//	if (rgbOut[i] > 0)
			//		cout << i << ": " << rgbOut[i] << endl;
			//)
			//cin.get();

			convertToFormatGray8(rgbOut, *(LF + num), sizeOut[_X], sizeOut[_Y], bytesperpixel);

			num++;

			int out = _findnext(ff, &data);
			if (out == -1)
				break;
		}
		_findclose(ff);
		cout << "LF load was successed." << endl;

		if (num_image[_X]*num_image[_Y] != num) {
			cout << "num_image is not matched." << endl;
			cin.get();
		}
		return 1;
	}
	else
	{
		cout << "LF load was failed." << endl;
		cin.get();
		return -1;
	}
	Complex<Real> one(1, 1);

	for (uint i = 0; i < 4000 * 4000; i++)
	{
		if (RSplane_complex_field[i] > one)
			cout << i << ": " << RSplane_complex_field[i] << endl;

	}
}

int ophLF::loadLF()
{
	initializeLF();

	_finddata_t data;

	string sdir = std::string("./").append(LF_directory).append("/").append("*.").append(ext);
	intptr_t ff = _findfirst(sdir.c_str(), &data);
	if (ff != -1)
	{
		int num = 0;
		uchar* rgbOut;
		ivec2 sizeOut;
		int bytesperpixel;

		while (1)
		{
			//for_i(5, cout << "before: " << *(*LF + num) << endl;);
			//cin.get();

			string imgfullname = std::string(LF_directory).append("/").append(data.name);

			getImgSize(sizeOut[_X], sizeOut[_Y], bytesperpixel, imgfullname.c_str());
			rgbOut = new uchar[sizeOut[_X] * sizeOut[_Y] * 3];

			rgbOut = loadAsImg(imgfullname.c_str());

			if (rgbOut == 0) {
				cout << "LF load was failed." << endl;
				cin.get();
				return -1;
			}


			//for_i(sizeOut[_X] * sizeOut[_Y] * 3,
			//	if (rgbOut[i] > 0)
			//		cout << i << ": " << rgbOut[i] << endl;
			//)
			//cin.get();

			convertToFormatGray8(rgbOut, *(LF + num), sizeOut[_X], sizeOut[_Y], bytesperpixel);

			num++;

			int out = _findnext(ff, &data);
			if (out == -1)
				break;
		}
		_findclose(ff);
		cout << "LF load was successed." << endl;

		if (num_image[_X] * num_image[_Y] != num) {
			cout << "num_image is not matched." << endl;
			cin.get();
		}
		return 1;
	}
	else
	{
		cout << "LF load was failed." << endl;
		cin.get();
		return -1;
	}
	Complex<Real> one(1, 1);

	for (uint i = 0; i < 4000 * 4000; i++)
	{
		if (RSplane_complex_field[i] > one)
			cout << i << ": " << RSplane_complex_field[i] << endl;

	}
}

void ophLF::generateHologram() {
	cout << "Generating Hologram..." << endl;
	convertLF2ComplexField();
	cout << "convert finished" << endl;
	fresnelPropagation(RSplane_complex_field, holo_gen, distanceRS2Holo);
	cout << "Hologram Generated.." << endl;
}


void ophLF::initializeLF() {
	//if (LF[0] != nullptr) {
	//	for_i(num_image[_X] * num_image[_Y],
	//		delete[] LF[i];);
	//}
	cout << "initialize LF..." << endl;

	initialize();

	LF = new uchar*[num_image[_X] * num_image[_Y]];

	for_i(num_image[_X] * num_image[_Y],
		LF[i] = new uchar[resolution_image[_X] * resolution_image[_Y]];);

	//for_i(num_image[_X] * num_image[_Y],
	//	oph::memsetArr<uchar>(*(LF + i), '0', 0, resolution_image[_X] * resolution_image[_Y]-1););

	cout << "The Number of the Images : " << num_image[_X] * num_image[_Y] << endl;
}


void ophLF::convertLF2ComplexField() {
	int nx = num_image[_X];
	int ny = num_image[_Y];
	int rx = resolution_image[_X];
	int ry = resolution_image[_Y];

	RSplane_complex_field = new Complex<Real>[nx*ny*rx*ry];

	Complex<Real>* complexLF = new Complex<Real>[rx*ry];

	Complex<Real>* FFTLF = new Complex<Real>[rx*ry];

	Real randVal;
	Complex<Real> phase;

	for (int idxRx = 0; idxRx < rx; idxRx++) {
		for (int idxRy = 0; idxRy < ry; idxRy++) {

			for (int idxNx = 0; idxNx < nx; idxNx++) {
				for (int idxNy = 0; idxNy < ny; idxNy++) {

					(*(complexLF + (idxNx + nx*idxNy)))._Val[_RE] = (Real)*(*(LF + (idxNx + nx*idxNy)) + (idxNx + nx*idxNy));
				}
			}
			
			fft2({ nx,ny }, complexLF, OPH_FORWARD, OPH_ESTIMATE);
			fftwShift(complexLF, FFTLF, nx, ny, OPH_FORWARD, false);
			
			for (int idxNx = 0; idxNx < nx; idxNx++) {
				for (int idxNy = 0; idxNy < ny; idxNy++) {

					randVal = rand((Real)0, (Real)1, idxRx*idxRy);
					phase._Val[_IM] = randVal * 2 * M_PI;

					*(RSplane_complex_field + nx*rx*ny*idxRy + nx*rx*idxNy + nx*idxRx + idxNx) = *(FFTLF+ (idxNx + nx*idxNy)) * phase.exp();
				}
			}		

		}
	}

	delete[] complexLF, FFTLF;
}