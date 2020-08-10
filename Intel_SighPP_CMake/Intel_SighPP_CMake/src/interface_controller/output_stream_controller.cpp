#pragma once

#include "librealsense2/rs.hpp"
#include "opencv2/opencv.hpp"
#include "../classification_result.hpp"

// TODO Should output stream window stuff be here or in a seperate class/file?
class OutputStreamController {
	
    //std::vector<PrioritisedClassificationResult> prioritised_results_;
    std::string depth_output_window_;
	bool show_depth_output_;
	
    std::string color_output_window_;
	bool show_color_output_;

    rs2::colorizer color_map_;

	// TODO Receive prioritised info from ServiceController
    // TODO Output something?

public:

	OutputStreamController(const bool show_depth_window, const bool show_color_window) : show_depth_output_(show_depth_window), show_color_output_(show_color_window)
	{
		std::cout << "Constructing output interface controller" << std::endl;
		if (show_depth_window)
		{
			depth_output_window_ = "Depth Image";
			cv::namedWindow(depth_output_window_, CV_WINDOW_NORMAL | CV_GUI_NORMAL);
		}
		if (show_color_window)
		{
			color_output_window_ = "Color Image";
			cv::namedWindow(color_output_window_, CV_WINDOW_NORMAL | CV_GUI_NORMAL);
		}
		std::cout << "Constructed output interface controller" << std::endl;
	}

	void stream_to_windows(const rs2::frame& depth_frame, cv::Mat depth_matrix, const rs2::video_frame& color_frame, cv::Mat color_matrix, const std::vector<PrioritisedClassificationResult>& vector) const
	{
		if (show_depth_output_)
		{
			depth_window(depth_frame);
		}

		if (show_color_output_)
		{
			color_window(color_matrix, depth_matrix, vector);
		}
	}
	
	void depth_window(const rs2::frame& frame) const
	{
        const auto width = frame.as<rs2::video_frame>().get_width();
        const auto height = frame.as<rs2::video_frame>().get_height();

		const cv::Mat depth_mat(cv::Size(width, height), CV_8UC3, const_cast<void*>(frame.get_data()), cv::Mat::AUTO_STEP);
        imshow(depth_output_window_, depth_mat);
    }

	void color_window(cv::Mat color_matrix, cv::Mat depth_matrix, const std::vector<PrioritisedClassificationResult>& vector) const
	{
        //const auto width = frame.as<rs2::video_frame>().get_width();
        //const auto height = frame.as<rs2::video_frame>().get_height();
		
		//const cv::Mat color_mat(cv::Size(width, height), CV_8UC3, const_cast<void*>(frame.get_data()), cv::Mat::AUTO_STEP);


        for (auto && prioritised_classification_result : vector)
        {
	        for (auto && item : prioritised_classification_result.objects)
	        {
				cv::Rect object(item.bottom_left.x, item.bottom_left.y, item.top_right.x - item.bottom_left.x, item.top_right.y - item.bottom_left.y);
				object = object & cv::Rect(0, 0, depth_matrix.cols, depth_matrix.rows);

				std::ostringstream ss;
				ss << item.name << " ";
				ss << std::setprecision(2) << item.distance << " meters away";
				cv::String conf(ss.str());

				rectangle(color_matrix, object, cv::Scalar(0, 255, 0));
				int base_line = 0;
				cv::Size label_size = cv::getTextSize(ss.str(), cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &base_line);

				auto center = (object.br() + object.tl()) * 0.5;
				center.x = center.x - label_size.width / 2;

				rectangle(
					color_matrix,
					cv::Rect(cv::Point(center.x, center.y - label_size.height),
					cv::Size(label_size.width, label_size.height + base_line)),
					cv::Scalar(255, 255, 255), cv::FILLED);

				cv::putText(color_matrix, ss.str(), center, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
	        }
        }
		
		// Switch RGB to BGR as openCV uses BGR
		cv::Mat bgr;
		cvtColor(color_matrix, bgr, cv::COLOR_RGB2BGR);
        cv::imshow(color_output_window_, bgr);
    }
	
	/// <summary>
	/// Tells the system if the opencv stream output windows are ready, if enabled.
	/// In the case where no output stream windows are enabled, returns true.
	/// Based on the rs-dnn and rs-imshow examples.
	/// </summary>
	/// <returns>
	///   Returns TRUE if the windows can receive frames, or if no output stream windows are enabled.
	///   Returns FALSE if the windows can not receive frames.
	/// </returns>
	bool should_receieve_new_frames() const
	{
		return wait_key() && is_depth_window_ready() && is_color_window_ready();
	}

	bool wait_key() const
	{
		if (show_depth_output_ || show_color_output_)
		{
			return cv::waitKey(1) < 0;
		}
		return true;
	}
	
	bool is_depth_window_ready() const
	{
		if (show_depth_output_)
		{
			return getWindowProperty(depth_output_window_, cv::WND_PROP_AUTOSIZE) >= 0;
		}
		return true;
	}

	bool is_color_window_ready() const
	{
		if (show_color_output_)
		{
			return getWindowProperty(color_output_window_, cv::WND_PROP_AUTOSIZE) >= 0;
		}
		return true;
	}

};